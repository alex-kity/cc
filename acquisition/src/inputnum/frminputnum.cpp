﻿#pragma execution_character_set("utf-8")

#include "frminputnum.h"
#include "ui_frminputnum.h"
#include "quihelper.h"

frmInputNum *frmInputNum::self = 0;
frmInputNum *frmInputNum::Instance()
{
    if (!self) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new frmInputNum;
        }
    }

    return self;
}

frmInputNum::frmInputNum(QWidget *parent) :	QWidget(parent), ui(new Ui::frmInputNum)
{
	ui->setupUi(this);
    this->initProperty();
    this->initForm();    
    this->changeStyle();
}

frmInputNum::~frmInputNum()
{
    delete ui;
}

void frmInputNum::init(QString style, int fontSize)
{
	this->currentStyle = style;
	this->currentFontSize = fontSize;
    this->changeStyle();
}

void frmInputNum::mouseMoveEvent(QMouseEvent *e)
{
	if (mousePressed && (e->buttons() && Qt::LeftButton)) {
		this->move(e->globalPos() - mousePoint);
		e->accept();
	}
}

void frmInputNum::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {
		mousePressed = true;
		mousePoint = e->globalPos() - this->pos();
		e->accept();
	}
}

void frmInputNum::mouseReleaseEvent(QMouseEvent *)
{
	mousePressed = false;
}

void frmInputNum::initProperty()
{
    ui->btn0->setProperty("btnNum", true);
    ui->btn1->setProperty("btnNum", true);
    ui->btn2->setProperty("btnNum", true);
    ui->btn3->setProperty("btnNum", true);
    ui->btn4->setProperty("btnNum", true);
    ui->btn5->setProperty("btnNum", true);
    ui->btn6->setProperty("btnNum", true);
    ui->btn7->setProperty("btnNum", true);
    ui->btn8->setProperty("btnNum", true);
    ui->btn9->setProperty("btnNum", true);
    ui->btn00->setProperty("btnNum", true);

    ui->btnDot->setProperty("btnOther", true);
    ui->btnSpace->setProperty("btnOther", true);
    ui->btnDelete->setProperty("btnOther", true);
}

void frmInputNum::initForm()
{
	this->mousePressed = false;
	this->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);


    deskWidth = QUIHelper::deskWidth();
    deskHeight = QUIHelper::deskHeight();
	frmWidth = this->width();
	frmHeight = this->height();

	isFirst = true;
	isPress = false;
	timerPress = new QTimer(this);
	connect(timerPress, SIGNAL(timeout()), this, SLOT(reClicked()));

    currentWidget = 0;
    currentLineEdit = 0;
    currentTextEdit = 0;
    currentPlain = 0;
    currentBrowser = 0;
    currentEditType = "";

    currentView = 0;
    currentRow = 0;
    currentColumn = 0;

	//如果需要更改输入法面板的样式,改变currentStyle这个变量即可
	//blue--淡蓝色  dev--dev风格  black--黑色  brown--灰黑色  lightgray--浅灰色  darkgray--深灰色  gray--灰色  silvery--银色
	currentStyle = "";

	//输入法面板字体大小,如果需要更改面板字体大小,该这里即可
	currentFontSize = 10;

	QList<QPushButton *> btn = this->findChildren<QPushButton *>();

	foreach (QPushButton *b, btn) {
		connect(b, SIGNAL(clicked()), this, SLOT(btnClicked()));
	}

	//绑定全局改变焦点信号槽
	connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)),
	        this, SLOT(focusChanged(QWidget *, QWidget *)));
	//绑定按键事件过滤器
	qApp->installEventFilter(this);
}

void frmInputNum::showPanel()
{
	this->setVisible(true);
}

void frmInputNum::focusChanged(QWidget *oldWidget, QWidget *nowWidget)
{
    //qDebug() << "oldWidget:" << oldWidget << "nowWidget:" << nowWidget;
	if (nowWidget != 0 && !this->isAncestorOf(nowWidget)) {
		//在Qt5和linux系统中(嵌入式linux除外),当输入法面板关闭时,焦点会变成无,然后焦点会再次移到焦点控件处
		//这样导致输入法面板的关闭按钮不起作用,关闭后马上有控件获取焦点又显示.
		//为此,增加判断,当焦点是从有对象转为无对象再转为有对象时不要显示.
		//这里又要多一个判断,万一首个窗体的第一个焦点就是落在可输入的对象中,则要过滤掉
#ifndef __arm__
		if (oldWidget == 0x0 && !isFirst) {
			QTimer::singleShot(0, this, SLOT(hide()));
			return;
		}
#endif

		//如果对应属性noinput为真则不显示
		if (nowWidget->property("noinput").toBool()) {
			QTimer::singleShot(0, this, SLOT(hide()));
			return;
		}

		isFirst = false;

        if (nowWidget->inherits("QExpandingLineEdit")) {
            QObject *obj = nowWidget->parent();
            if (obj != 0) {
                obj = obj->parent();
                if (obj != 0) {
                    currentView = (QTableView *)obj;
                    currentRow = currentView->currentIndex().row();
                    currentColumn = currentView->currentIndex().column();
                    currentEditType = "QExpandingLineEdit";
                    showPanel();
                }
            }
        } else if (nowWidget->inherits("QLineEdit")) {
			currentLineEdit = (QLineEdit *)nowWidget;
			currentEditType = "QLineEdit";
            showPanel();
		} else if (nowWidget->inherits("QTextEdit")) {
			currentTextEdit = (QTextEdit *)nowWidget;
			currentEditType = "QTextEdit";
            showPanel();
		} else if (nowWidget->inherits("QPlainTextEdit")) {
			currentPlain = (QPlainTextEdit *)nowWidget;
			currentEditType = "QPlainTextEdit";
            showPanel();
		} else if (nowWidget->inherits("QTextBrowser")) {
			currentBrowser = (QTextBrowser *)nowWidget;
			currentEditType = "QTextBrowser";
            showPanel();
		} else if (nowWidget->inherits("QComboBox")) {
			QComboBox *cbox = (QComboBox *)nowWidget;

			//只有当下拉选择框处于编辑模式才可以输入
			if (cbox->isEditable()) {
				currentLineEdit = cbox->lineEdit() ;
				currentEditType = "QLineEdit";
                showPanel();
			}
		} else if (nowWidget->inherits("QSpinBox") ||
		           nowWidget->inherits("QDoubleSpinBox") ||
		           nowWidget->inherits("QDateEdit") ||
		           nowWidget->inherits("QTimeEdit") ||
		           nowWidget->inherits("QDateTimeEdit")) {
			currentWidget = nowWidget;
			currentEditType = "QWidget";
            showPanel();
		} else {
			currentWidget = 0;
			currentLineEdit = 0;
			currentTextEdit = 0;
			currentPlain = 0;
			currentBrowser = 0;
			currentEditType = "";
            currentView = 0;
            currentRow = 0;
            currentColumn = 0;
			this->setVisible(false);
		}

		QRect rect = nowWidget->rect();
		QPoint pos = QPoint(rect.left(), rect.bottom() + 2);
		pos = nowWidget->mapToGlobal(pos);

		int x = pos.x();
		int y = pos.y();

		if (pos.x() + frmWidth > deskWidth) {
			x = deskWidth - frmWidth;
		}

		if (pos.y() + frmHeight > deskHeight) {
			y = pos.y() - frmHeight - rect.height() - 2;
		}

		this->setGeometry(x, y, frmWidth, frmHeight);
	}
}

bool frmInputNum::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		if (currentEditType != "") {
			if (obj != ui->btnClose) {
                QString objName = obj->objectName();
                if (obj->parent() != 0x0 && !obj->property("noinput").toBool() && objName != "frmMainWindow"
                        && objName != "frmInputWindow" && objName != "qt_edit_menu" && objName != "labPY") {
                    if (obj->inherits("QGroupBox") || obj->inherits("QFrame") || obj->inherits("QMenu")) {
                        this->hide();
                    } else {
                        showPanel();
                    }
                }
			}

			btnPress = (QPushButton *)obj;
			if (checkPress()) {
				isPress = true;
				timerPress->start(500);
			}
		}

		return false;
	} else if (event->type() == QEvent::MouseButtonRelease) {
		btnPress = (QPushButton *)obj;
		if (checkPress()) {
			isPress = false;
			timerPress->stop();
		}

		return false;
	}

	return QWidget::eventFilter(obj, event);
}

bool frmInputNum::checkPress()
{
	//只有属于数字键盘的合法按钮才继续处理
	bool num_ok = btnPress->property("btnNum").toBool();
	bool other_ok = btnPress->property("btnOther").toBool();

	if (num_ok || other_ok) {
		return true;
	}

	return false;
}

void frmInputNum::reClicked()
{
	if (isPress) {
		timerPress->setInterval(30);
		btnPress->click();
	}
}

void frmInputNum::btnClicked()
{
	//如果当前焦点控件类型为空,则返回不需要继续处理
	if (currentEditType == "") {
		return;
	}

	QPushButton *btn = (QPushButton *)sender();
	QString objectName = btn->objectName();

	if (objectName == "btnDelete") {
		deleteValue();
	} else if (objectName == "btnClose") {
		this->setVisible(false);
	} else if (objectName == "btnEnter") {
		this->setVisible(false);
	} else if (objectName == "btnSpace") {
		insertValue(" ");
    } else if (objectName == "btnEnter") {
        insertValue("\n");
    } else {
		QString value = btn->text();
		insertValue(value);
	}
}
#include <qvariant.h>
#include <qstringbuilder.h>
void frmInputNum::insertValue(QString value)
{
	if (currentEditType == "QLineEdit") {
		currentLineEdit->insert(value);
	} else if (currentEditType == "QTextEdit") {
		currentTextEdit->insertPlainText(value);
	} else if (currentEditType == "QPlainTextEdit") {
		currentPlain->insertPlainText(value);
	} else if (currentEditType == "QTextBrowser") {
		currentBrowser->insertPlainText(value);
	} else if (currentEditType == "QWidget") {
		QKeyEvent keyPress(QEvent::KeyPress, 0, Qt::NoModifier, QString(value));
		QApplication::sendEvent(currentWidget, &keyPress);
    } else if (currentEditType == "QExpandingLineEdit") {
        QAbstractItemModel *model = currentView->model();
        QModelIndex index = model->index(currentRow, currentColumn);
        QString str = model->data(index).toString();
        str = str + value;
        model->setData(index, str);
    }
}

void frmInputNum::deleteValue()
{
	if (currentEditType == "QLineEdit") {
		currentLineEdit->backspace();
	} else if (currentEditType == "QTextEdit") {
		//获取当前QTextEdit光标,如果光标有选中,则移除选中字符,否则删除光标前一个字符
		QTextCursor cursor = currentTextEdit->textCursor();

		if (cursor.hasSelection()) {
			cursor.removeSelectedText();
		} else {
			cursor.deletePreviousChar();
		}
	} else if (currentEditType == "QPlainTextEdit") {
		//获取当前QTextEdit光标,如果光标有选中,则移除选中字符,否则删除光标前一个字符
		QTextCursor cursor = currentPlain->textCursor();

		if (cursor.hasSelection()) {
			cursor.removeSelectedText();
		} else {
			cursor.deletePreviousChar();
		}
	} else if (currentEditType == "QTextBrowser") {
		//获取当前QTextEdit光标,如果光标有选中,则移除选中字符,否则删除光标前一个字符
		QTextCursor cursor = currentBrowser->textCursor();

		if (cursor.hasSelection()) {
			cursor.removeSelectedText();
		} else {
			cursor.deletePreviousChar();
		}
	} else if (currentEditType == "QWidget") {
		QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QString());
		QApplication::sendEvent(currentWidget, &keyPress);
    } else if (currentEditType == "QExpandingLineEdit") {
        QAbstractItemModel *model = currentView->model();
        QModelIndex index = model->index(currentRow, currentColumn);
        QString str = model->data(index).toString();
        model->setData(index, str.mid(0, str.length() - 1));
    }
}

void frmInputNum::changeStyle()
{
	if (currentStyle == "blue") {
		changeStyle("#DEF0FE", "#C0DEF6", "#C0DCF2", "#386487");
	} else if (currentStyle == "dev") {
		changeStyle("#C0D3EB", "#BCCFE7", "#B4C2D7", "#324C6C");
	} else if (currentStyle == "gray") {
		changeStyle("#E4E4E4", "#A2A2A2", "#A9A9A9", "#000000");
	} else if (currentStyle == "lightgray") {
		changeStyle("#EEEEEE", "#E5E5E5", "#D4D0C8", "#6F6F6F");
	} else if (currentStyle == "darkgray") {
		changeStyle("#D8D9DE", "#C8C8D0", "#A9ACB5", "#5D5C6C");
	} else if (currentStyle == "black") {
		changeStyle("#4D4D4D", "#292929", "#D9D9D9", "#CACAD0");
	} else if (currentStyle == "brown") {
		changeStyle("#667481", "#566373", "#C2CCD8", "#E7ECF0");
	} else if (currentStyle == "silvery") {
		changeStyle("#E1E4E6", "#CCD3D9", "#B2B6B9", "#000000");
	}
}

void frmInputNum::changeStyle(QString topColor, QString bottomColor, QString borderColor, QString textColor)
{
	QStringList qss;
	qss.append(QString("QWidget#widget_title{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
	           .arg(topColor).arg(bottomColor));
	qss.append("QPushButton{padding:5px;border-radius:3px;}");
	qss.append(QString("QPushButton:hover{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
	           .arg(topColor).arg(bottomColor));
	qss.append(QString("QLabel,QPushButton{font-size:%1pt;color:%2;}")
	           .arg(currentFontSize).arg(textColor));
	qss.append(QString("QPushButton#btnPre,QPushButton#btnNext,QPushButton#btnClose{padding:5px;}"));
	qss.append(QString("QPushButton{border:1px solid %1;background:rgba(0,0,0,0);}")
	           .arg(borderColor));
	qss.append(QString("QLineEdit{border:1px solid %1;border-radius:5px;padding:2px;background:none;selection-background-color:%2;selection-color:%3;}")
	           .arg(borderColor).arg(bottomColor).arg(topColor));
	this->setStyleSheet(qss.join(""));
}
