/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <fep3/fep3_result_decl.h>

#include <functional>
#include <memory>
#include <vector>

///@cond nodoc
namespace fep3::core::detail {

struct NoAction {
};

struct IAction {
    virtual fep3::Result doAction() = 0;
    virtual void undoAction() = 0;
    virtual ~IAction() = default;
};
template <typename Do, typename Undo>
struct ActionPair : IAction {
    ActionPair(Do&& doA, Undo&& undoA) : _doa(doA), _undoa(undoA)
    {
    }
    fep3::Result doAction()
    {
        using RetType = decltype(_doa());
        if constexpr (std::is_convertible_v<RetType, fep3::Result>) {
            return std::invoke(_doa);
        }
        else {
            std::invoke(_doa);
            return {};
        }
    }
    void undoAction()
    {
        if constexpr (!std::is_same_v<Undo, NoAction>) {
            std::invoke(_undoa);
        }
    }

private:
    Do _doa;
    Undo _undoa;
};

template <typename Do, typename Undo>
std::unique_ptr<IAction> getIaction(Do&& doA, Undo&& undoA)
{
    return std::make_unique<ActionPair<Do, Undo>>(std::forward<Do>(doA), std::forward<Undo>(undoA));
}

struct Transition {
    template <typename Do, typename Undo>
    void addAction(Do&& doA, Undo&& undoA)
    {
        _iactions.emplace_back(getIaction(std::forward<Do>(doA), std::forward<Undo>(undoA)));
    }

    fep3::Result doTransition()
    {
        fep3::Result ret;

        auto action_iterator = _iactions.begin();
        while (action_iterator != _iactions.end()) {
            ret = (*action_iterator)->doAction();
            if (!ret)
                break;
            ++action_iterator;
        }

        if (action_iterator != _iactions.end()) {
            doReverseTransition(std::make_reverse_iterator(action_iterator));
        }

        return ret;
    }

    void doReverseTransition()
    {
        doReverseTransition(_iactions.rbegin());
    }

private:
    template <typename ReverseIterator>
    void doReverseTransition(ReverseIterator it_rev)
    {
        while (it_rev != _iactions.rend()) {
            (*it_rev)->undoAction();
            ++it_rev;
        }
    }

    std::vector<std::unique_ptr<IAction>> _iactions;
};

} // namespace fep3::core::detail

///@endcond nodoc
