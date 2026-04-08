// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef DATATYPE_H
#define DATATYPE_H

// Enum with the different data types that can be stored in the Model class.
// The DataType of the object is stored in JsonDiff to be able to undo/redo
// changes. The registerModel() method of the UndoStack class links the DataType
// to the right Model instance.
enum class DataType {
    InputParameter,
    OutputParameter,
    Correlation
};

template <typename T>
DataType dataTypeFor() = delete;

#endif // DATATYPE_H
