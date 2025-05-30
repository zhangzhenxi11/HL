#include "ExcelAssistant/excel_assistant.h"

#include <QFile>
#include <QDebug>
#include <QDir>

ExcelAssistant::ExcelAssistant()
{

}
ExcelAssistant::~ExcelAssistant()
{
	if (excel)
	{
		//workBooks->dynamicCall("Close()");
		//excel->dynamicCall("Quit()"); //退出进程
		workBook->dynamicCall("Close(Boolean)", false); //! 关闭exce程序先关闭.xls文件
		workBooks->dynamicCall("Close()");
		excel->dynamicCall("Quit(void)");
		delete excel;
	}
}
QList<QList<QVariant>> ExcelAssistant::openFile(const QString file_path){
	if (!excel)
	{
		excel = new QAxObject();
		if (excel->setControl("Excel.Application")) 
		{
		
		}
		else
		{
			excel->setControl("ket.Application");  // 加载 WPS Excel 控件
		}
		excel->setProperty("Visible", false);  // 不显示 Excel 窗体
		excel->setProperty("DisplayAlerts", false); //! 不显示任何警告信息， 如关闭时的是否保存提示
	}
	if (!workBooks)
	{
		workBooks = excel->querySubObject("WorkBooks");  //获取工作簿集合
	}
	workBooks->dynamicCall("Open(const QString&)", QDir::toNativeSeparators(file_path)); //打开打开已存在的工作簿
	workBook = excel->querySubObject("ActiveWorkBook"); //获取当前工作簿
	//QAxObject* sheets = workBook->querySubObject("Sheets");  //获取工作表集合，Sheets也可换用WorkSheets
	QAxObject* sheet = workBook->querySubObject("WorkSheets(int)", 1);//获取工作表集合的工作表1，即sheet1
	//获取该sheet的使用范围对象（一般读取 Excel 时会选取全部范围）
	QAxObject* usedRange = sheet->querySubObject("UsedRange");
	QVariant var = usedRange->dynamicCall("Value");
	delete usedRange;
	QList<QList<QVariant>> excel_list;
	QVariantList varRows = var.toList();
	if (!varRows.isEmpty())
	{
		for (auto row : varRows)
		{
			excel_list.append(row.toList());
		}

	}
	workBook->dynamicCall("Close(Boolean)", false); //关闭exce程序先关闭.xls文件
	workBooks->dynamicCall("Close()");
	excel->dynamicCall("Quit(void)");
	return excel_list;

}

bool ExcelAssistant::saveFile(const QList<QList<QVariant> > data, const QString file_path)
{
	if (!excel)
	{
		excel = new QAxObject();
		if (excel->setControl("Excel.Application")) {}
		else
		{
			excel->setControl("ket.Application");  // 加载 WPS Excel 控件
		}
		excel->setProperty("Visible", false);  // 不显示 Excel 窗体
		excel->setProperty("DisplayAlerts", false); //不显示任何警告信息，如关闭时的是否保存提示
	}
	if (!workBooks)
	{
		workBooks = excel->querySubObject("WorkBooks");  //获取工作簿集合
	}
	QFile file(file_path);
	if (file.exists())
	{
		workBooks->dynamicCall("Open(const QString&)", QDir::toNativeSeparators(file_path)); //打开已存在的工作簿
	}
	else
	{
		workBooks->dynamicCall("Add"); //新建一个工作簿
	}

	workBook = excel->querySubObject("ActiveWorkBook"); // 获取当前工作簿
	QString col_e, targetRange;

	convertToColName(data[0].size(), col_e);
	//qDebug() << data[0].size();
	targetRange = QString("A1:%1%2").arg(col_e).arg(QString::number(data.size()));
	//qDebug() << targetRange;
	QAxObject* sheet = workBook->querySubObject("WorkSheets(int)", 1);//获取工作表集合的工作表1，即sheet1
	QAxObject* usedRange = sheet->querySubObject("Range(const QString)", targetRange);
	//usedRange->setProperty("NumberFormat", "@");    //  设置所有单元格为文本属性
	QVariantList vars;
	const int rows = data.size();      //获取行数
	for (int i = 0; i < rows; ++i)
	{
		vars.append(QVariant(data[i]));    //将list(i)添加到QVariantList中 QVariant(cells[i])强制转换
	}

	usedRange->dynamicCall("SetValue(const QVariant&", QVariant(vars));
	workBook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(file_path));
	//workBook->dynamicCall("Save()");	// 保存文件
	delete usedRange;
	workBook->dynamicCall("Close(Boolean)", false); //关闭exce程序先关闭.xls文件
	workBooks->dynamicCall("Close()");
	excel->dynamicCall("Quit(void)");
	return true;
}
QString ExcelAssistant::to26AlphabetString(int data)
{
	QChar ch = (QChar)(data + 0x40);//A对应0x41
	return QString(ch);
}


void ExcelAssistant::convertToColName(int data, QString &res)
{
	Q_ASSERT(data > 0 && data < 65535);
	int tempData = data / 26;
	if (tempData > 0)
	{
		int mode = data % 26;
		QString col_name1 = to26AlphabetString(tempData);
		QString col_name2 = to26AlphabetString(mode); \
			res = col_name1 + col_name2;

	}
	else {
		res = to26AlphabetString(data);
	}
}

