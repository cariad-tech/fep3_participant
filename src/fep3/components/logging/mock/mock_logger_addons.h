/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/logging/mock_logger.h>

namespace fep3 {
namespace mock {

struct LoggerWithDefaultBehavior : public Logger {
    LoggerWithDefaultBehavior()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled()).WillByDefault(Return(true));
    }
};

// Logger mock which simulates not being configured for any logging severity
// and therefore logs nothing
struct InactiveLogger : public Logger {
    InactiveLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isErrorEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isFatalEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isDebugEnabled()).WillByDefault(Return(false));
    }
};

// Logger mock which simulates to be configured for warning, error and fatal logging severities
// and therefore logs the corresponding messages
struct WarningLogger : public Logger {
    WarningLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled()).WillByDefault(Return(false));
    }
};

// Logger mock which simulates to be configured for error and fatal logging severities
// and therefore logs the corresponding messages
struct ErrorLogger : public Logger {
    ErrorLogger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isWarningEnabled()).WillByDefault(Return(false));
        ON_CALL(*this, isErrorEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled()).WillByDefault(Return(false));
    }
};

struct LoggerWithDefaultBehaviour : public LoggerWithDefaultBehavior {
    LoggerWithDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, logInfo(_)).WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logWarning(_)).WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logError(_)).WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logFatal(_)).WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logDebug(_)).WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, isInfoEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled()).WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled()).WillByDefault(Return(true));
    }
};

} // namespace mock
} // namespace fep3
