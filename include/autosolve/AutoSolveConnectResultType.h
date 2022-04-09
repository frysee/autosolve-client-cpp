#pragma once

namespace AutoSolveClient {
  enum AutoSolveConnectResultType {
    InvalidClientId,
    InvalidApiKeyOrAccessToken,
    Success,
    UnknownError
  };
}
