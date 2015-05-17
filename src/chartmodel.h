#ifndef CHARTMODEL_H
#define CHARTMODEL_H

#include <QObject>
#include <QTimer>

class ChartData {

};

class ChartModel : public QObject
{
    Q_OBJECT
public:
    explicit ChartModel(QObject *parent = 0);
    Q_PROPERTY(QList<int> dataset1 READ dataset1 NOTIFY dataset1Changed())
    QList<int> dataset1();
private:
    QList<int> mDataset1;
private slots:
    void updateDataset1();
signals:
    void dataset1Changed();
};

#endif // CHARTMODEL_H
