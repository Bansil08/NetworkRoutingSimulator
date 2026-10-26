#pragma once

// Outcome of a core operation. The core never prints; the UI layer turns
// a Status into a user-facing message.
enum class Status {
    Ok,
    EmptyRouterName,
    DuplicateRouter,
    SourceNotFound,
    DestinationNotFound,
    SelfLoop,
    NegativeWeight,
    DuplicateLink,
    LinkNotFound,
    EmptyNetwork,
    FileOpenFailed,
    MalformedLine,
};
