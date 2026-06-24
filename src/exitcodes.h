// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef EXITCODES_H
#define EXITCODES_H

// The exit codes used in the Sigma app
enum class ExitCode {
    Success         = 0,
    UsageError      = 1,    // Invalid command line argument(s)
    LoadError       = 2,    // Failed to load project
    SaveError       = 3,    // Failed to save project
    SimulationError = 4,    // Monte Carlo simulation failed
    ExportError     = 5,    // Failed to write CSV export file
    Interrupted     = 130,  // SIGINT - Ctrl+C on command-line
    QmlLoadFailure  = 255,  // Abnormal termination due to QML failure
};

#endif // EXITCODES_H
