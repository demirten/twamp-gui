#include "chartmodel.h"

ChartModel::ChartModel(QObject *parent) : QObject(parent)
{
    QTimer::singleShot(1000, this, SLOT(updateDataset1()));
}

QList<int> ChartModel::dataset1()
{
    return mDataset1;
}

void ChartModel::updateDataset1()
{
    mDataset1.append(qrand() % 100);
    QTimer::singleShot(1000, this, SLOT(updateDataset1()));
}
