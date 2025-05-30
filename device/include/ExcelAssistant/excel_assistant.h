#ifndef EXCEL_ASSISTANT_H
#define EXCEL_ASSISTANT_H
#include <QString>
#include <QAxObject>
#include <QVariant>
#include <QVariantList>

class ExcelAssistant
{
public:
    ExcelAssistant();
    ~ExcelAssistant();
public:
    QList<QList<QVariant>> openFile(const QString file_path);
    bool saveFile(const QList<QList<QVariant>> data,const QString file_path);
private:
    QString to26AlphabetString(int data);
    void convertToColName(int data, QString &res);
private:
    QAxObject *excel = 0;
    QAxObject* workBooks = 0;
    QAxObject* workBook = 0;
};

#endif // EXCEL_ASSISTANT_H
