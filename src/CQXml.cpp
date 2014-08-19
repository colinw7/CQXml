#include <CQXml.h>
#include <CQStyleWidget.h>
#include <CXML.h>
#include <CXMLToken.h>
#include <CStrUtil.h>

#include <QCalendarWidget>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDial>
#include <QDialog>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QFontComboBox>
#include <QGroupBox>
#include <QLCDNumber>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QWebView>
#include <QWizard>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

#include <QMetaProperty>

#include <iostream>
#include <cassert>

namespace CQXmlUtil {
  QBoxLayout::Direction stringToBoxLayoutDirection(const QString &str) {
    if      (str.toLower() == "lefttoright") return QBoxLayout::LeftToRight;
    else if (str.toLower() == "righttoleft") return QBoxLayout::RightToLeft;
    else if (str.toLower() == "toptobottom") return QBoxLayout::TopToBottom;
    else if (str.toLower() == "bottomtotop") return QBoxLayout::BottomToTop;
    else                                     return QBoxLayout::LeftToRight;
  }

  QBoxLayout *newBoxLayout(QWidget *w, const QString &str) {
    return new QBoxLayout(stringToBoxLayoutDirection(str), w);
  }
}

using namespace CQXmlUtil;

class CQXmlRootTag;

class CQXmlFactory : public CXMLFactory {
 public:
  CQXmlFactory(CQXml *xml) :
   CXMLFactory(), xml_(xml) {
  }

  CQXml *getXml() const { return xml_; }

  void setRoot(CQXmlRootTag *root) { root_ = root; }

  CXMLTag *createTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  void createWidgets(QWidget *parent);

  void createWidgets(CXMLTag *tag, QLayout *layout);
  void createWidgets(CXMLTag *tag, QWidget *widget);

 private:
  CQXml        *xml_;
  CQXmlRootTag *root_;
};

template<typename T>
class QtWidgetFactory : public CQXmlWidgetFactory {
 public:
  QtWidgetFactory() { }

  QWidget *createWidget(const QStringList &) {
    return new T;
  }
};

class CQXmlTag : public CXMLTag {
 public:
  CQXmlTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CXMLTag(parent, name, options) {
  }

  virtual ~CQXmlTag() { }

  CQXml *getXml() const;

  virtual bool isRoot  () const { return false; }
  virtual bool isLayout() const { return false; }
  virtual bool isWidget() const { return false; }
  virtual bool isExec  () const { return false; }

  virtual QLayout *createLayout(QWidget *, QLayout *, CQXmlTag *) { return 0; }

  virtual QWidget *createWidget(QLayout *, CQXmlTag *) { return 0; }
  virtual QWidget *createWidget(QWidget *, CQXmlTag *) { return 0; }

  virtual void exec() { }

  virtual void endLayout() { }

  virtual void handleOptions(CXMLTag::OptionArray &options) {
    CXMLTag::OptionArray::const_iterator o1, o2;

    for (auto option : options) {
      const std::string &name  = option->getName();
      const std::string &value = option->getValue();

      if (! handleOption(name, value))
        nameValues_[name.c_str()] = value.c_str();
    }
  }

  bool hasNameValue(const QString &name) const {
    return (nameValues_.find(name) != nameValues_.end());
  }

  QString nameValue(const QString &name) const {
    NameValues::const_iterator p = nameValues_.find(name);

    if (p != nameValues_.end())
      return (*p).second;
    else
      return "";
  }

  virtual bool handleOption(const std::string &, const std::string &) { return false; }

  QString getText() const {
    std::string text = CXMLTag::getText(false);

    return (! text.length() ? nameValue("text") : QString(text.c_str()));
  }

 protected:
  typedef std::map<QString,QString> NameValues;

  NameValues nameValues_;
};

class CQXmlLayoutTag : public CQXmlTag {
 public:
  enum Type  {
   HBox,
   VBox,
   Box,
   Grid,
   Form
  };

 public:
  CQXmlLayoutTag(CXMLTag *parent, Type type, const std::string &name,
                 CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), type_(type) {
  }

  bool isLayout() const { return true; }

  QLayout *createLayout(QWidget *w, QLayout *l, CQXmlTag *) {
    if      (type_ == HBox) layout_ = new QHBoxLayout(w);
    else if (type_ == VBox) layout_ = new QVBoxLayout(w);
    else if (type_ == Grid) layout_ = new QGridLayout(w);
    else if (type_ == Form) layout_ = new QFormLayout(w);
    else if (type_ == Box ) layout_ = newBoxLayout(w, nameValue("direction"));
    else                    assert(false);

    int margin = 2, spacing = 2;

    if (hasNameValue("margin" )) margin  = nameValue("margin" ).toInt();
    if (hasNameValue("spacing")) spacing = nameValue("spacing").toInt();

    layout_->setMargin (margin );
    layout_->setSpacing(spacing);

    if (l) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout  *>(l)->addLayout(layout_);
      else if (qobject_cast<QGridLayout *>(l))
        qobject_cast<QGridLayout *>(l)->addLayout(layout_, 0, 0);
      else if (qobject_cast<QFormLayout *>(l)) {
        QString label = nameValue("formLabel");

        qobject_cast<QFormLayout *>(l)->addRow(label, layout_);
      }
      else
        assert(false);
    }

    if (hasNameValue("name"))
      getXml()->addLayout(nameValue("name"), layout_);

    return layout_;
  }

  bool handleOption(const std::string &name, const std::string &value) {
    if      (name == "columnStretch") {
      std::vector<std::string> fields;

      CStrUtil::addFields(value, fields, ",");

      if (fields.size() == 2)
        columnStretches_.push_back(IntIntPair(std::stoi(fields[0]), std::stoi(fields[1])));
    }
    else if (name == "rowStretch") {
      std::vector<std::string> fields;

      CStrUtil::addFields(value, fields, ",");

      if (fields.size() == 2)
        rowStretches_.push_back(IntIntPair(std::stoi(fields[0]), std::stoi(fields[1])));
    }
    else
      return false;

    return true;
  }

  void endLayout() {
    if      (qobject_cast<QBoxLayout *>(layout_)) {
    }
    else if (qobject_cast<QGridLayout *>(layout_)) {
      QGridLayout *grid = qobject_cast<QGridLayout *>(layout_);

      for (auto ipair : columnStretches_)
        grid->setColumnStretch(ipair.first, ipair.second);

      for (auto ipair : rowStretches_)
        grid->setRowStretch(ipair.first, ipair.second);
    }
    else if (qobject_cast<QFormLayout *>(layout_)) {
    }
    else
      assert(false);
  }

 private:
  typedef std::pair<int,int>      IntIntPair;
  typedef std::vector<IntIntPair> IntIntPairArray;

  Type            type_;
  QLayout*        layout_;
  IntIntPairArray columnStretches_;
  IntIntPairArray rowStretches_;
};

class CQXmlRootTag : public CQXmlTag {
 public:
  CQXmlRootTag(CXMLTag *parent, CQXml *xml, const std::string &name,
               CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), xml_(xml), type_(CQXmlLayoutTag::VBox) {
    CQXmlFactory *factory = xml->getFactory();

    factory->setRoot(this);

    for (auto option : options) {
      const std::string &name  = option->getName();
      const std::string &value = option->getValue();

      if (name == "layoutType") {
        QString value1(value.c_str());

        if      (value1.toLower() == "hbox") type_ = CQXmlLayoutTag::HBox;
        else if (value1.toLower() == "vbox") type_ = CQXmlLayoutTag::VBox;
        else if (value1.toLower() == "box" ) type_ = CQXmlLayoutTag::Box;
        else if (value1.toLower() == "grid") type_ = CQXmlLayoutTag::Grid;
        else if (value1.toLower() == "form") type_ = CQXmlLayoutTag::Form;
      }
    }
  }

  bool isRoot() const { return true; }

  CQXml *getXml() const { return xml_; }

  bool handleOption(const std::string &name, const std::string &value) {
    if (name == "windowTitle")
      getXml()->parent()->setWindowTitle(value.c_str());
    else
      return false;

    return true;
  }

  QLayout *createLayout(QWidget *parent) {
    QLayout *layout = 0;

    if      (type_ == CQXmlLayoutTag::HBox) layout = new QHBoxLayout(parent);
    else if (type_ == CQXmlLayoutTag::VBox) layout = new QVBoxLayout(parent);
    else if (type_ == CQXmlLayoutTag::Box ) layout = newBoxLayout(parent, nameValue("direction"));
    else if (type_ == CQXmlLayoutTag::Grid) layout = new QGridLayout(parent);
    else if (type_ == CQXmlLayoutTag::Form) layout = new QFormLayout(parent);

    layout->setMargin(0); layout->setSpacing(0);

    return layout;
  }

 private:
  CQXml                *xml_;
  CQXmlLayoutTag::Type  type_;
};

class CQXmlHeadingTag : public CQXmlTag {
 public:
  CQXmlHeadingTag(CXMLTag *parent, int level, const std::string &name,
                  CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), level_(level) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QLayout *l, CQXmlTag *) {
    QString type = QString("h%1").arg(level_);

    return CQStyleWidgetMgrInst->addStyleLabel(l, getText(), type);
  }

 private:
  int level_;
};

class CQXmlPTag : public CQXmlTag {
 public:
  CQXmlPTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QLayout *l, CQXmlTag *) {
    return CQStyleWidgetMgrInst->addParagraph(l, getText());
  }
};

class CQXmlLayoutItemTag : public CQXmlTag {
 public:
  CQXmlLayoutItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isLayout() const { return true; }

  QLayout *createLayout(QWidget *, QLayout *l, CQXmlTag *) {
    if      (qobject_cast<QBoxLayout *>(l)) {
      if (hasNameValue("spacing"))
        qobject_cast<QBoxLayout *>(l)->addSpacing(nameValue("spacing").toInt());

      if (hasNameValue("stretch"))
        qobject_cast<QBoxLayout *>(l)->addStretch(nameValue("stretch").toInt());
    }

    return l;
  }
};

class CQXmlComboItemTag : public CQXmlTag {
 public:
  CQXmlComboItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QComboBox *>(w)) return w;

    if (hasNameValue("icon")) {
      QPixmap pixmap(nameValue("icon"));

      qobject_cast<QComboBox *>(w)->addItem(QIcon(pixmap), getText());
    }
    else
      qobject_cast<QComboBox *>(w)->addItem(getText());

    return w;
  }
};

class CQXmlListItemTag : public CQXmlTag {
 public:
  CQXmlListItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QListWidget *>(w)) return w;

    qobject_cast<QListWidget *>(w)->addItem(getText());

    return w;
  }
};

class CQXmlTableItemTag : public CQXmlTag {
 public:
  CQXmlTableItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QTableWidget *>(w)) return w;

    QTableWidgetItem *item = new QTableWidgetItem(getText());

    int row = nameValue("row"   ).toInt();
    int col = nameValue("column").toInt();

    qobject_cast<QTableWidget *>(w)->setItem(row, col, item);

    return w;
  }
};

class CQXmlTreeItemTag : public CQXmlTag {
 public:
  CQXmlTreeItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QTreeWidget *>(w)) return w;

    QStringList items = getText().split(' ');

    QTreeWidgetItem *item = new QTreeWidgetItem(items);

    qobject_cast<QTreeWidget *>(w)->addTopLevelItem(item);

    return w;
  }
};

class CQXmlTabItemTag : public CQXmlTag {
 public:
  CQXmlTabItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QTabBar *>(w)) return w;

    if (hasNameValue("icon")) {
      QPixmap pixmap(nameValue("icon"));

      qobject_cast<QTabBar *>(w)->addTab(QIcon(pixmap), getText());
    }
    else
      qobject_cast<QTabBar *>(w)->addTab(getText());

    return w;
  }
};

class CQXmlMenuTitleTag : public CQXmlTag {
 public:
  CQXmlMenuTitleTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    if (! qobject_cast<QMenuBar *>(w)) return w;

    QMenu *menu = qobject_cast<QMenuBar *>(w)->addMenu(getText());

    return menu;
  }
};

class CQXmlActionTag : public CQXmlTag {
 public:
  CQXmlActionTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    QAction *action = 0;

    if      (hasNameValue("actionRef"))
      action = getXml()->getAction(nameValue("actionRef"));
    else if (hasNameValue("icon")) {
      QPixmap pixmap(nameValue("icon"));

      action = new QAction(QIcon(pixmap), getText(), 0);
    }
    else if (getText().length())
      action = new QAction(getText(), 0);

    if (! action)
      return w;

    if      (qobject_cast<QMenu *>(w))
      qobject_cast<QMenu *>(w)->addAction(action);
    else if (qobject_cast<QToolBar *>(w))
      qobject_cast<QToolBar *>(w)->addAction(action);

    if (hasNameValue("name"))
      getXml()->addAction(nameValue("name"), action);

    return w;
  }
};

class CQXmlConnectTag : public CQXmlTag {
 public:
  CQXmlConnectTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  bool isExec() const { return true; }

  void exec() {
    QWidget *source = 0, *dest = 0;

    if (hasNameValue("source"))
      source = getXml()->getWidget(nameValue("source"));

    if (hasNameValue("dest"))
      dest = getXml()->getWidget(nameValue("dest"));

    QString sourceSignal = "2" + nameValue("sourceSignal");

    if (hasNameValue("destSignal")) {
      QString destSignal = "2" + nameValue("destSignal");

      QObject::connect(source, sourceSignal.toLatin1(), dest, destSignal.toLatin1());
    }
    else {
      QString destSlot = "1" + nameValue("destSlot");

      QObject::connect(source, sourceSignal.toLatin1(), dest, destSlot.toLatin1());
    }
  }
};

class CQXmlQtWidgetTag : public CQXmlTag {
 public:
  CQXmlQtWidgetTag(CXMLTag *parent, const std::string &type, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, type, options), type_(type.c_str()) {
  }

  bool isWidget() const { return true; }

  QWidget *createWidget(QLayout *l, CQXmlTag *) {
    QString text = getText();

    QWidget *w = createWidgetI(text);

    bool allowLayout = true;

    if (qobject_cast<QDialog *>(w) ||
        qobject_cast<QMainWindow *>(w) ||
        qobject_cast<QMenu *>(w))
      allowLayout = false;

    if (l && allowLayout) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout *>(l)->addWidget(w);
      else if (qobject_cast<QGridLayout *>(l)) {
        int row = nameValue("row").toInt();
        int col = nameValue("col").toInt();

        qobject_cast<QGridLayout *>(l)->addWidget(w, row, col);
      }
      else if (qobject_cast<QFormLayout *>(l)) {
        QString label = nameValue("formLabel");

        qobject_cast<QFormLayout *>(l)->addRow(label, w);
      }
    }

    if (hasNameValue("menuRef")) {
      QMenu *menu = qobject_cast<QMenu *>(getXml()->getWidget(nameValue("menuRef")));
      if (! menu) return w;

      if      (qobject_cast<QToolButton *>(w))
        qobject_cast<QToolButton *>(w)->setMenu(menu);
      else if (qobject_cast<QPushButton *>(w))
        qobject_cast<QPushButton *>(w)->setMenu(menu);
    }

    if (qobject_cast<QDialog *>(w) || qobject_cast<QMainWindow *>(w))
      w->show();

    return w;
  }

  QWidget *createWidget(QWidget *w, CQXmlTag *) {
    QString text = getText();

    QWidget *w1 = createWidgetI(text);

    if      (qobject_cast<QTabWidget *>(w)) {
      QString text1 = nameValue("tabText");

      if (hasNameValue("tabIcon")) {
        QPixmap pixmap(nameValue("tabIcon"));

        qobject_cast<QTabWidget *>(w)->addTab(w1, QIcon(pixmap), text1);
      }
      else
        qobject_cast<QTabWidget *>(w)->addTab(w1, text1);
    }
    else if (qobject_cast<QToolBox *>(w)) {
      QString text1 = nameValue("toolText");

      if (hasNameValue("toolIcon")) {
        QPixmap pixmap(nameValue("toolIcon"));

        qobject_cast<QToolBox *>(w)->addItem(w1, QIcon(pixmap), text1);
      }
      else
        qobject_cast<QToolBox *>(w)->addItem(w1, text1);
    }
    else if (qobject_cast<QMenuBar *>(w)) {
      if (qobject_cast<QMenu *>(w1)) {
        qobject_cast<QMenuBar *>(w)->addMenu(qobject_cast<QMenu *>(w1));
      }
    }
    else if (qobject_cast<QMenu *>(w)) {
      if (qobject_cast<QMenu *>(w1)) {
        qobject_cast<QMenu *>(w)->addMenu(qobject_cast<QMenu *>(w1));
      }
    }
    else if (qobject_cast<QMainWindow *>(w)) {
      if      (qobject_cast<QDockWidget *>(w1)) {
        Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;

        QString dockWidgetArea = nameValue("dockWidgetArea");

        if (dockWidgetArea.length()) {
          if      (dockWidgetArea.toLower() == "left"  ) area = Qt::LeftDockWidgetArea;
          else if (dockWidgetArea.toLower() == "right" ) area = Qt::RightDockWidgetArea;
          else if (dockWidgetArea.toLower() == "top"   ) area = Qt::TopDockWidgetArea;
          else if (dockWidgetArea.toLower() == "bottom") area = Qt::BottomDockWidgetArea;
        }

        qobject_cast<QMainWindow *>(w)->addDockWidget(area, qobject_cast<QDockWidget *>(w1));
      }
      else if (qobject_cast<QToolBar *>(w1)) {
        Qt::ToolBarArea area = Qt::TopToolBarArea;

        QString toolBarArea = nameValue("toolBarArea");

        if (toolBarArea.length()) {
          if      (toolBarArea.toLower() == "left"  ) area = Qt::LeftToolBarArea;
          else if (toolBarArea.toLower() == "right" ) area = Qt::RightToolBarArea;
          else if (toolBarArea.toLower() == "top"   ) area = Qt::TopToolBarArea;
          else if (toolBarArea.toLower() == "bottom") area = Qt::BottomToolBarArea;
        }

        qobject_cast<QMainWindow *>(w)->addToolBar(area, qobject_cast<QToolBar *>(w1));
      }
      else if (qobject_cast<QMenuBar *>(w1))
        qobject_cast<QMainWindow *>(w)->setMenuBar(qobject_cast<QMenuBar *>(w1));
      else if (qobject_cast<QStatusBar *>(w1))
        qobject_cast<QMainWindow *>(w)->setStatusBar(qobject_cast<QStatusBar *>(w1));
      else if (qobject_cast<QMdiArea *>(w1))
        qobject_cast<QMainWindow *>(w)->setCentralWidget(qobject_cast<QMdiArea *>(w1));
      else
        qobject_cast<QMainWindow *>(w)->setCentralWidget(w1);
    }
    else if (qobject_cast<QDockWidget *>(w)) {
      qobject_cast<QDockWidget *>(w)->setWidget(w1);
    }
    else if (qobject_cast<QMdiArea *>(w)) {
      qobject_cast<QMdiArea *>(w)->addSubWindow(w1);
    }
    else if (qobject_cast<QMdiSubWindow *>(w)) {
      qobject_cast<QMdiSubWindow *>(w)->setWidget(w1);
    }
    else if (qobject_cast<QWizard *>(w)) {
      qobject_cast<QWizard *>(w)->addPage(qobject_cast<QWizardPage *>(w1));
    }

    return w1;
  }

 private:
  QWidget *createWidgetI(const QString &text) {
    CQXml *xml = getXml();

    CQXmlWidgetFactory *factory = xml->getWidgetFactory(type_);

    QWidget *w = factory->createWidget(options_);

    if (hasNameValue("name")) {
      w->setObjectName(nameValue("name"));

      getXml()->addWidget(nameValue("name"), w);
    }

    if      (qobject_cast<QLabel *>(w))
      qobject_cast<QLabel *>(w)->setText(text);
    else if (qobject_cast<QAbstractButton *>(w))
      qobject_cast<QAbstractButton *>(w)->setText(text);
    else if (qobject_cast<QLineEdit *>(w))
      qobject_cast<QLineEdit *>(w)->setText(text);
    else if (qobject_cast<QPlainTextEdit *>(w))
      qobject_cast<QPlainTextEdit *>(w)->setPlainText(text);
    else if (qobject_cast<QTextEdit *>(w))
      qobject_cast<QTextEdit *>(w)->setText(text);

    const QMetaObject *meta = w->metaObject();

    if (meta) {
      for (auto nameValue : nameValues_) {
        int propIndex = meta->indexOfProperty(nameValue.first.toLatin1());
        if (propIndex < 0) continue;

        QMetaProperty mP = meta->property(propIndex);
        if (! mP.isWritable()) continue;

        if (mP.isEnumType()) {
          QString name(nameValue.second);

          QMetaEnum me = mP.enumerator();

          for (int i = 0; i < me.keyCount(); ++i) {
            if (me.key(i) == name)
              (void) w->setProperty(nameValue.first.toLatin1(), me.value(i));
          }
        }
        else {
          QVariant v(nameValue.second);

          if      (mP.type() == QVariant::Icon) {
            QPixmap pixmap(nameValue.second);

            v = QIcon(pixmap);
          }
          else if (mP.type() == QVariant::Pixmap) {
            QPixmap pixmap(nameValue.second);

            v = pixmap;
          }
          else {
            if (! v.convert(mP.type()))
              continue;
          }

          (void) w->setProperty(nameValue.first.toLatin1(), v);
        }
      }
    }

    if      (qobject_cast<QTableWidget *>(w)) {
      QStringList columnLabels = nameValue("columnLabels").split(' ');

      if (columnLabels.length())
        qobject_cast<QTableWidget *>(w)->setHorizontalHeaderLabels(columnLabels);

      QStringList rowLabels = nameValue("rowLabels").split(' ');

      if (rowLabels.length())
        qobject_cast<QTableWidget *>(w)->setVerticalHeaderLabels(rowLabels);
    }
    else if (qobject_cast<QTreeWidget *>(w)) {
      QStringList columnLabels = nameValue("columnLabels").split(' ');

      if (columnLabels.length())
        qobject_cast<QTreeWidget *>(w)->setHeaderLabels(columnLabels);
    }

    if       (hasNameValue("minimumSize")) {
      QStringList sizes = nameValue("minimumSize").split(' ');

      if (sizes.length() == 2) {
        int w1 = sizes[0].toInt();
        int w2 = sizes[1].toInt();

        w->setMinimumSize(QSize(w1, w2));
      }
    }
    else if (hasNameValue("minimumWidth")) {
      int w1 = nameValue("minimumWidth").toInt();

      w->setMinimumWidth(w1);
    }
    else if (hasNameValue("maximumHeight")) {
      int h1 = nameValue("maximumHeight").toInt();

      w->setMinimumHeight(h1);
    }

    if (hasNameValue("maximumSize")) {
      QStringList sizes = nameValue("maximumSize").split(' ');

      if (sizes.length() == 2) {
        int h1 = sizes[0].toInt();
        int h2 = sizes[1].toInt();

        w->setMaximumSize(QSize(h1, h2));
      }
    }
    else if (hasNameValue("maximumWidth")) {
      int w1 = nameValue("manimumWidth").toInt();

      w->setMaximumWidth(w1);
    }
    else if (hasNameValue("maximumHeight")) {
      int h1 = nameValue("manimumHeight").toInt();

      w->setMaximumHeight(h1);
    }

    return w;
  }

 private:
  QString     type_;
  QStringList options_;
};

//------

CQXml::
CQXml() :
 parent_(0)
{
  xml_ = new CXML;

  factory_ = new CQXmlFactory(this);

  xml_->setFactory(factory_);

  addWidgetFactory("QCalendarWidget", new QtWidgetFactory<QCalendarWidget>());
  addWidgetFactory("QCheckBox"      , new QtWidgetFactory<QCheckBox>      ());
  addWidgetFactory("QColorDialog"   , new QtWidgetFactory<QColorDialog>   ());
  addWidgetFactory("QComboBox"      , new QtWidgetFactory<QComboBox>      ());
  addWidgetFactory("QDial"          , new QtWidgetFactory<QDial>          ());
  addWidgetFactory("QDialog"        , new QtWidgetFactory<QDialog>        ());
  addWidgetFactory("QDateEdit"      , new QtWidgetFactory<QDateEdit>      ());
  addWidgetFactory("QDateTimeEdit"  , new QtWidgetFactory<QDateTimeEdit>  ());
  addWidgetFactory("QDockWidget"    , new QtWidgetFactory<QDockWidget>    ());
  addWidgetFactory("QDoubleSpinBox" , new QtWidgetFactory<QDoubleSpinBox> ());
  addWidgetFactory("QFileDialog"    , new QtWidgetFactory<QFileDialog>    ());
  addWidgetFactory("QFontComboBox"  , new QtWidgetFactory<QFontComboBox>  ());
  addWidgetFactory("QFontDialog"    , new QtWidgetFactory<QFontDialog>    ());
  addWidgetFactory("QFrame"         , new QtWidgetFactory<QFrame>         ());
  addWidgetFactory("QGroupBox"      , new QtWidgetFactory<QGroupBox>      ());
  addWidgetFactory("QLabel"         , new QtWidgetFactory<QLabel>         ());
  addWidgetFactory("QLCDNumber"     , new QtWidgetFactory<QLCDNumber>     ());
  addWidgetFactory("QLineEdit"      , new QtWidgetFactory<QLineEdit>      ());
  addWidgetFactory("QListView"      , new QtWidgetFactory<QListView>      ());
  addWidgetFactory("QListWidget"    , new QtWidgetFactory<QListWidget>    ());
  addWidgetFactory("QMainWindow"    , new QtWidgetFactory<QMainWindow>    ());
  addWidgetFactory("QMdiArea"       , new QtWidgetFactory<QMdiArea>       ());
  addWidgetFactory("QMdiSubWindow"  , new QtWidgetFactory<QMdiSubWindow>  ());
  addWidgetFactory("QMenu"          , new QtWidgetFactory<QMenu>          ());
  addWidgetFactory("QMenuBar"       , new QtWidgetFactory<QMenuBar>       ());
  addWidgetFactory("QMessageBox"    , new QtWidgetFactory<QMessageBox>    ());
  addWidgetFactory("QPlainTextEdit" , new QtWidgetFactory<QPlainTextEdit> ());
  addWidgetFactory("QPrintDialog"   , new QtWidgetFactory<QPrintDialog>   ());
  addWidgetFactory("QProgressBar"   , new QtWidgetFactory<QProgressBar>   ());
  addWidgetFactory("QProgressDialog", new QtWidgetFactory<QProgressDialog>());
  addWidgetFactory("QPushButton"    , new QtWidgetFactory<QPushButton>    ());
  addWidgetFactory("QRadioButton"   , new QtWidgetFactory<QRadioButton>   ());
  addWidgetFactory("QScrollArea"    , new QtWidgetFactory<QScrollArea>    ());
  addWidgetFactory("QScrollBar"     , new QtWidgetFactory<QScrollBar>     ());
  addWidgetFactory("QSlider"        , new QtWidgetFactory<QSlider>        ());
  addWidgetFactory("QSpinBox"       , new QtWidgetFactory<QSpinBox>       ());
  addWidgetFactory("QSplitter"      , new QtWidgetFactory<QSplitter>      ());
  addWidgetFactory("QStackedWidget" , new QtWidgetFactory<QStackedWidget> ());
  addWidgetFactory("QStatusBar"     , new QtWidgetFactory<QStatusBar>     ());
  addWidgetFactory("QTabBar"        , new QtWidgetFactory<QTabBar>        ());
  addWidgetFactory("QTabWidget"     , new QtWidgetFactory<QTabWidget>     ());
  addWidgetFactory("QTableView"     , new QtWidgetFactory<QTableView>     ());
  addWidgetFactory("QTableWidget"   , new QtWidgetFactory<QTableWidget>   ());
  addWidgetFactory("QTextEdit"      , new QtWidgetFactory<QTextEdit>      ());
  addWidgetFactory("QToolBar"       , new QtWidgetFactory<QToolBar>       ());
  addWidgetFactory("QToolBox"       , new QtWidgetFactory<QToolBox>       ());
  addWidgetFactory("QToolButton"    , new QtWidgetFactory<QToolButton>    ());
  addWidgetFactory("QTreeView"      , new QtWidgetFactory<QTreeView>      ());
  addWidgetFactory("QTreeWidget"    , new QtWidgetFactory<QTreeWidget>    ());
  addWidgetFactory("QWidget"        , new QtWidgetFactory<QWidget>        ());
  addWidgetFactory("QWebView"       , new QtWidgetFactory<QWebView>       ());
  addWidgetFactory("QWizard"        , new QtWidgetFactory<QWizard>        ());
  addWidgetFactory("QWizardPage"    , new QtWidgetFactory<QWizardPage>    ());
}

CQXml::
~CQXml()
{
  delete xml_;
}

bool
CQXml::
isWidgetFactory(const QString &name) const
{
  return (widgetFactories_.find(name) != widgetFactories_.end());
}

void
CQXml::
addWidgetFactory(const QString &name, CQXmlWidgetFactory *factory)
{
  if (isWidgetFactory(name))
    removeWidgetFactory(name);

  widgetFactories_[name] = factory;
}

void
CQXml::
removeWidgetFactory(const QString &name)
{
  WidgetFactories::iterator p = widgetFactories_.find(name);
  assert(p != widgetFactories_.end());

  widgetFactories_.erase(p);
}

CQXmlWidgetFactory *
CQXml::
getWidgetFactory(const QString &name) const
{
  WidgetFactories::const_iterator p = widgetFactories_.find(name);
  assert(p != widgetFactories_.end());

  return (*p).second;
}

void
CQXml::
createWidgetsFromString(QWidget *parent, const std::string &str)
{
  parent_ = parent;

  CXMLTag *tag;

  if (! xml_->readString(str, &tag))
    return;

  factory_->createWidgets(parent);
}

void
CQXml::
createWidgetsFromFile(QWidget *parent, const std::string &filename)
{
  parent_ = parent;

  CXMLTag *tag;

  if (! xml_->read(filename, &tag))
    return;

  factory_->createWidgets(parent);
}

void
CQXml::
addLayout(const QString &name, QLayout *l)
{
  layouts_[name] = l;
}

QLayout *
CQXml::
getLayout(const QString &name) const
{
  LayoutMap::const_iterator p = layouts_.find(name);

  if (p == layouts_.end())
    return 0;

  return (*p).second;
}

void
CQXml::
addWidget(const QString &name, QWidget *w)
{
  widgets_[name] = w;
}

QWidget *
CQXml::
getWidget(const QString &name) const
{
  WidgetMap::const_iterator p = widgets_.find(name);

  if (p == widgets_.end())
    return 0;

  return (*p).second;
}

void
CQXml::
addAction(const QString &name, QAction *w)
{
  actions_[name] = w;
}

QAction *
CQXml::
getAction(const QString &name) const
{
  ActionMap::const_iterator p = actions_.find(name);

  if (p == actions_.end())
    return 0;

  return (*p).second;
}

//-------

CXMLTag *
CQXmlFactory::
createTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options)
{
  CQXmlTag *tag = 0;

  if      (name == "qxml")
    tag = new CQXmlRootTag(parent, xml_, name, options);
  else if (name == "QHBoxLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::HBox, name, options);
  else if (name == "QVBoxLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::VBox, name, options);
  else if (name == "QBoxLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::Box, name, options);
  else if (name == "QGridLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::Grid, name, options);
  else if (name == "QFormLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::Form, name, options);
  else if (name == "QLayoutItem")
    tag = new CQXmlLayoutItemTag(parent, name, options);
  else if (name == "QComboItem")
    tag = new CQXmlComboItemTag(parent, name, options);
  else if (name == "QListItem")
    tag = new CQXmlListItemTag(parent, name, options);
  else if (name == "QTableItem")
    tag = new CQXmlTableItemTag(parent, name, options);
  else if (name == "QTreeItem")
    tag = new CQXmlTreeItemTag(parent, name, options);
  else if (name == "QTabItem")
    tag = new CQXmlTabItemTag(parent, name, options);
  else if (name == "QMenuTitle")
    tag = new CQXmlMenuTitleTag(parent, name, options);
  else if (name == "QAction")
    tag = new CQXmlActionTag(parent, name, options);
  else if (name == "connect")
    tag = new CQXmlConnectTag(parent, name, options);
  else if (xml_->isWidgetFactory(name.c_str()))
    tag = new CQXmlQtWidgetTag(parent, name, options);
  else if (name == "h1")
    tag = new CQXmlHeadingTag(parent, 1, name, options);
  else if (name == "h2")
    tag = new CQXmlHeadingTag(parent, 2, name, options);
  else if (name == "h3")
    tag = new CQXmlHeadingTag(parent, 3, name, options);
  else if (name == "h4")
    tag = new CQXmlHeadingTag(parent, 4, name, options);
  else if (name == "p")
    tag = new CQXmlPTag(parent, name, options);
  else {
    std::cerr << "Invalid tag name " << name << std::endl;
    return CXMLFactory::createTag(parent, name, options);
  }

  if (tag)
    tag->handleOptions(options);

  return tag;
}

void
CQXmlFactory::
createWidgets(QWidget *parent)
{
  QLayout *layout = root_->createLayout(parent);

  createWidgets(root_, layout);
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QLayout *layout)
{
  CQXmlTag *ptag = dynamic_cast<CQXmlTag *>(tag);

  for (int i = 0; i < tag->getNumChildren(); ++i) {
    const CXMLToken *token = tag->getChild(i);

    if (token->isTag()) {
      CQXmlTag *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if      (tag1->isLayout()) {
        QLayout *layout1 = tag1->createLayout(0, layout, ptag);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else if (tag1->isWidget()) {
        QWidget *widget = tag1->createWidget(layout, ptag);

        createWidgets(tag1, widget);
      }
      else if (tag1->isExec()) {
        tag1->exec();
      }
    }
  }
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QWidget *widget)
{
  CQXmlTag *ptag = dynamic_cast<CQXmlTag *>(tag);

  for (int i = 0; i < tag->getNumChildren(); ++i) {
    const CXMLToken *token = tag->getChild(i);

    if (token->isTag()) {
      CQXmlTag *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if      (tag1->isLayout()) {
        QLayout *layout1 = tag1->createLayout(widget, 0, ptag);

        if (qobject_cast<QGroupBox *>(widget))
          qobject_cast<QGroupBox *>(widget)->setLayout(layout1);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else if (tag1->isWidget()) {
        QWidget *widget1 = tag1->createWidget(widget, ptag);

        createWidgets(tag1, widget1);
      }
      else if (tag1->isExec()) {
        tag1->exec();
      }
    }
  }
}

//------

CQXml *
CQXmlTag::
getXml() const
{
  CQXmlTag *parent = dynamic_cast<CQXmlTag *>(getParent());

  while (parent && ! parent->isRoot())
    parent = dynamic_cast<CQXmlTag *>(parent->getParent());

  if (parent)
    return dynamic_cast<CQXmlRootTag *>(parent)->getXml();

  return 0;
}
