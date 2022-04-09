#pragma once

#include "AutoSolveMessage.h"
#include <QList>

namespace AutoSolveClient
{
    class AutoSolveCancelResponse
    {
        QList<AutoSolveMessage *> m_requests;

    public:
        AutoSolveCancelResponse() = default;

        QList<AutoSolveMessage *> &requests() { return m_requests; }
        const QList<AutoSolveMessage *> &requests() const { return m_requests; }
        void setRequests(const QList<AutoSolveMessage *> &requests) { m_requests = requests; }
    };
}
