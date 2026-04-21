// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef CORRELATION_H
#define CORRELATION_H

#include "data.h"
#include "datatype.h"
#include "inputparameter.h"
#include "jsondiff.h"
#include "modelcontrol.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1StringView>
#include <QList>
#include <QObject>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QtQmlIntegration/qqmlintegration.h>

// Class that stores the correlation coefficient between 2 InputParameters and
// defines the static ModelControl object that stores the correlations
class Correlation : public QObject, public Data {
    Q_OBJECT
    QML_ELEMENT

public:
    Correlation(
        QObject *parent = nullptr,
        InputParameter *inputParameterA = nullptr,
        InputParameter *inputParameterB = nullptr,
        double correlation = 0.
    );
    Correlation( const Correlation &cor );

    Correlation & operator= ( const Correlation &cor );
    bool operator== ( const Correlation &cor ) const;
    bool operator!= ( const Correlation &cor ) const;

    Correlation *appendToModel();
    DataType dataType() const override;
    InputParameter *getInputParameterA() const;
    InputParameter *getInputParameterB() const;
    QJsonObject toJson() const override;
    QString getName( bool csvMode = false ) const override;
    QString toCSVString() const;
    QVariant get( int column, bool csvMode = false ) const override;
    QVariant headerData( int column ) const override;
    QUuid getInputParameterAId() const;
    QUuid getInputParameterBId() const;
    bool getValid() const override;
    int columnCount() const override;
    void reset();
    void set( int column, const QVariant &value ) override;
    void setInputParameterA( InputParameter *inputParameter = nullptr );
    void setInputParameterAById( const QUuid &id );
    void setInputParameterAId( const QUuid &id );
    void setInputParameterB( InputParameter *inputParameter = nullptr );
    void setInputParameterBById( const QUuid &id );
    void setInputParameterBId( const QUuid &id );
    void updateFromJson( const QJsonObject &json ) override;

    Q_INVOKABLE QString getInputParameterNameA( bool csvMode = false ) const;
    Q_INVOKABLE QString getInputParameterNameB( bool csvMode = false ) const;
    Q_INVOKABLE bool isUnique( bool checkCurrentSelection = false ) const;
    Q_INVOKABLE bool setInputParameterAByName( const QString &name );
    Q_INVOKABLE bool setInputParameterBByName( const QString &name );
    Q_INVOKABLE double getCorrelation() const;
    Q_INVOKABLE void setCorrelation( double correlation = 0. );
    Q_INVOKABLE void setToSelected();

    static Correlation *getById( const QUuid &id );
    static Correlation *getCorrelation( const QUuid &idA, const QUuid &idB );
    static Correlation *getSelected();
    static Correlation fromJson(
        const QJsonObject &json,
        bool appendToModel = true,
        QObject *parent = nullptr
    );
    static ModelControl<Correlation *> *getCorrelationModel();
    static QJsonArray correlationsToJson();
    static QJsonObject currentJson( const QUuid &id );
    static const QList<Correlation *> &getAll();
    static QList<Correlation *> getCorrelationsForInputParameter(
        const QUuid &id
    );
    static QString correlationsToCSVString();
    static QUuid getSelectedId();
    static bool correlationIsUnique(
        const Correlation *newCorrelation,
        bool checkCurrentSelection = false
    );
    static bool remove(  const QUuid &id );
    static bool update( const QUuid &id, Correlation *correlation );
    static int getRowIndex( const QUuid &id );
    static void applyDiff( const JsonDiff &diff );
    static void clearModel();
    static void correlationsFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );
    static void notifyInputParameterChanged( const QUuid &id );
    static void onDisplayPrecisionChanged();
    static void reconnectAllCorrelations();
    static void setSelectionLocked( bool locked );

    inline const static QList<int> columnWidths { 155, 155, 158 };
    inline const static QStringList headerLabels {
        "Input parameter A",
        "Input parameter B",
        "Correlation coefficient"
    };

private:
    Correlation *insertIntoModel( int row );
    void reconnectInputParameters();

    static bool sameInputParameters(
        const Correlation *corrA,
        const Correlation *corrB
    );

    InputParameter *mInputParameterA;
    InputParameter *mInputParameterB;
    QUuid mInputParameterAId;
    QUuid mInputParameterBId;
    double mCorrelation;

    static ModelControl<Correlation *> mCorrelationModel;
    static constexpr QLatin1StringView sCorrelationsHeaderString {
        "Correlations:"
    };
    static constexpr QLatin1StringView sCorrelationString { "correlation" };
    static constexpr QLatin1StringView sIdInputAString { "IdInputParameterA" };
    static constexpr QLatin1StringView sIdInputBString { "IdInputParameterB" };
};

#endif // CORRELATION_H
