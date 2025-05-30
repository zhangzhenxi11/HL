// Library: VTM
// Package: VTM

#include "ScientificDoubleSpinBox.h"



	QString ScientificDoubleSpinBox::textFromValue(double value) const {
		//logInform("SaveConfig", "textFromValue %.6f", value);
		return QString::number(value, 'e', decimals());
	}

    double ScientificDoubleSpinBox::valueFromText(const QString &text) const 
	{
		bool ok;
		double value = text.toDouble(&ok);
		if (!ok)
		{
			return this->value(); // 如果解析失败，返回当前值
		}
		return value;
	}
	void ScientificDoubleSpinBox::focusOutEvent(QFocusEvent *event)
	{
		QDoubleSpinBox::focusOutEvent(event);
		// 确保在失去焦点时更新显示文本
		//lineEdit()->setText(textFromValue(value()));
		updateText();
	}
	void ScientificDoubleSpinBox::keyPressEvent(QKeyEvent *event) 
	{
		QDoubleSpinBox::keyPressEvent(event);
		//logInform("SaveConfig", "keyPressEvent");
		updateText();
		//QTimer::singleShot(0, this, &ScientificDoubleSpinBox::updateText);
	}
	void ScientificDoubleSpinBox::onEditingFinished()
	{
		QString text = lineEdit()->text();
		bool ok;
		double value = text.toDouble(&ok);
		//logInform("SaveConfig", "onEditingFinished %.8f %d %s", value, ok, text.toStdString());
		if (ok)
		{
			setValue(value);
		}
		else
		{
			// 如果输入无效，恢复到之前的有效值
			lineEdit()->setText(textFromValue(this->value()));
		}
	}

	void ScientificDoubleSpinBox::onValueChanged(double value)
	{
		//logInform("SaveConfig", "onValueChanged %.8f", value);
		lineEdit()->setText(textFromValue(value));
	}

	void ScientificDoubleSpinBox::updateText()
	{
		QString text = lineEdit()->text();
		QRegExp regExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]{2,})$");
		if (regExp.exactMatch(text)){
			bool ok;
			double value = text.toDouble(&ok);
			//logInform("SaveConfig", "updateText %.8f %d %s", value, ok, text.toStdString());
			if (ok)
			{
				blockSignals(true);
				setValue(value);
				blockSignals(false);
			}
		}
		
	}