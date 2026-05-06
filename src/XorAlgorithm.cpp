#include "XorAlgorithm.h"

void XorAlgorithm::apply(QByteArray &data, const std::array<quint8, 8> &key, quint64 offset)
{
    for (qsizetype i = 0; i < data.size(); ++i) {
        const qsizetype keyIndex = static_cast<qsizetype>((offset + static_cast<quint64>(i)) % key.size());
        data[i] = static_cast<char>(static_cast<uchar>(data[i]) ^ key[static_cast<size_t>(keyIndex)]);
    }
}
