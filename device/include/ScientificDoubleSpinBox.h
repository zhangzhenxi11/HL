
#include <QWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QRegExp>
#include <QRegExpValidator>
#include <QHBoxLayout>
#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QTimer>
#include "Kernel/kernel_log.h"


class ScientificLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	ScientificLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

protected:
	void focusOutEvent(QFocusEvent *event) override
	{
		QLineEdit::focusOutEvent(event);
		emit editingFinished();  // 确保在失去焦点时发送编辑完成信号
	}
};

	class ScientificDoubleSpinBox : public QDoubleSpinBox {
		Q_OBJECT
	public:
		explicit ScientificDoubleSpinBox(QWidget *parent = nullptr) : QDoubleSpinBox(parent)
		{
			// 使用自定义的 QLineEdit
			setLineEdit(new ScientificLineEdit(this));
			// 设置自定义的校验器
			QRegExp regExp("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
			QRegExpValidator *validator = new QRegExpValidator(regExp, this);
			lineEdit()->setValidator(validator);

			// 初始化显示格式
			setDecimals(8); // 设置小数位数

			connect(lineEdit(), &QLineEdit::editingFinished, this, &ScientificDoubleSpinBox::onEditingFinished);
			connect(this, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
		}

	protected:
		QString textFromValue(double value) const override;
		virtual double valueFromText(const QString &text) const override;
		void focusOutEvent(QFocusEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;
		void updateText();

	private slots:
	void onEditingFinished();
	void onValueChanged(double value);

};