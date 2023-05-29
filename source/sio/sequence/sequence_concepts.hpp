/*
 * Copyright (c) 2023 Maikel Nadolski
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <exec/env.hpp>
#include <exec/sequence_senders.hpp>

namespace exec {
  namespace sequence_queries {
    using namespace stdexec;

    inline struct unbounded_t {
    } unbounded;

    struct cardinality_t {
      template <class Env>
        requires(!tag_invocable<cardinality_t, const Env&>)
      constexpr unbounded_t operator()(const Env&) const noexcept {
        return {};
      }

      template <class Env>
        requires tag_invocable<cardinality_t, const Env&>
      constexpr tag_invoke_result_t<cardinality_t, const Env&> operator()(const Env& env) const
        noexcept(nothrow_tag_invocable<cardinality_t, const Env&>) {
        return tag_invoke(*this, env);
      }
    };

    inline struct many_sender_t {
    } many_sender;

    inline struct lock_step_t {
    } lock_step;

    struct parallelism_t {
      template <class Env>
        requires(!tag_invocable<parallelism_t, const Env&>)
      constexpr many_sender_t operator()(const Env&) const noexcept {
        return {};
      }

      template <class Env>
        requires tag_invocable<parallelism_t, const Env&>
      constexpr tag_invoke_result_t<parallelism_t, const Env&> operator()(const Env& env) const
        noexcept(nothrow_tag_invocable<parallelism_t, const Env&>) {
        return tag_invoke(*this, env);
      }
    };
  }

  using sequence_queries::cardinality_t;
  inline constexpr cardinality_t cardinality;

  using sequence_queries::parallelism_t;
  inline constexpr parallelism_t parallelism;

  using sequence_queries::unbounded_t;
  using sequence_queries::unbounded;

  using sequence_queries::lock_step_t;
  using sequence_queries::lock_step;

  using sequence_queries::many_sender_t;
  using sequence_queries::many_sender;

  STDEXEC_DEFINE_CPO(struct get_sequence_env_t, get_sequence_env) {
    template <sequence_sender Sequence>
    requires tag_invocable<get_sequence_env_t, const Sequence&>
    constexpr tag_invoke_result_t<get_sequence_env_t,const Sequence&>
    operator()(const Sequence& seq)
      const noexcept {
      static_assert(nothrow_tag_invocable<get_sequence_env_t, const Sequence&>);
      return tag_invoke(*this, seq);
    }

    template <sequence_sender Sequence>
      requires(!tag_invocable<get_sequence_env_t, const Sequence&>)
    constexpr empty_env operator()(const Sequence& seq) const noexcept {
      return {};
    }

    template <sender Sequence>
      requires(!sequence_sender<Sequence>)
    constexpr auto operator()(const Sequence& seq) const noexcept {
      return make_env(
        with(cardinality_t{}, std::integral_constant<size_t, 1>{}),
        with(parallelism_t{}, lock_step));
    }
  };

  inline constexpr get_sequence_env_t get_sequence_env;

  template <class EnvProvider>
  using sequence_env_of_t = stdexec::__call_result_t<get_sequence_env_t, EnvProvider>;

  template <class Sequence, class Receiver>
  concept nothrow_subscribeable =
    stdexec::__nothrow_callable<exec::subscribe_t, Sequence, Receiver>;
}
