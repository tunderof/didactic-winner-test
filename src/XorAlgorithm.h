#pragma once

#include <QByteArray>
#include <QtGlobal>

#include <array>

class XorAlgorithm
{
public:
    static void apply(QByteArray &data, const std::array<quint8, 8> &key, quint64 offset);
};
