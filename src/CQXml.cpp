#include <CQXml.h>
#include <CXML.h>
#include <CXMLToken.h>
#include <CStrUtil.h>

#include <CQStyleWidget.h>
#include <CQPropertyTree.h>

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
#include <QUndoView>
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
  enum LayoutType {
   HBoxLayout,
   VBoxLayout,
   BoxLayout,
   GridLayout,
   FormLayout,
   NoLayout
  };

  QBoxLayout::Direction stringToBoxLayoutDirection(const QString &str) {
    if      (str.toLower() == "lefttoright") return QBoxLayout::LeftToRight;
    else if (str.toLower() == "righttoleft") return QBoxLayout::RightToLeft;
    else if (str.toLower() == "toptobottom") return QBoxLayout::TopToBottom;
    else if (str.toLower() == "bottomtotop") return QBoxLayout::BottomToTop;
    else                                     return QBoxLayout::LeftToRight;
  }

  LayoutType stringToLayoutType(const QString &str) {
    if      (str.toLower() == "hbox") return CQXmlUtil::HBoxLayout;
    else if (str.toLower() == "vbox") return CQXmlUtil::VBoxLayout;
    else if (str.toLower() == "box" ) return CQXmlUtil::BoxLayout;
    else if (str.toLower() == "grid") return CQXmlUtil::GridLayout;
    else if (str.toLower() == "form") return CQXmlUtil::FormLayout;
    else if (str.toLower() == "none") return CQXmlUtil::NoLayout;
    else                              return CQXmlUtil::VBoxLayout;
  }

  Qt::DockWidgetArea stringToDockWidgetArea(const QString &str) {
    if      (str.toLower() == "left"  ) return Qt::LeftDockWidgetArea;
    else if (str.toLower() == "right" ) return Qt::RightDockWidgetArea;
    else if (str.toLower() == "top"   ) return Qt::TopDockWidgetArea;
    else if (str.toLower() == "bottom") return Qt::BottomDockWidgetArea;
    else                                return Qt::RightDockWidgetArea;
  }

  Qt::ToolBarArea stringToToolBarArea(const QString &str) {
    if      (str.toLower() == "left"  ) return Qt::LeftToolBarArea;
    else if (str.toLower() == "right" ) return Qt::RightToolBarArea;
    else if (str.toLower() == "top"   ) return Qt::TopToolBarArea;
    else if (str.toLower() == "bottom") return Qt::BottomToolBarArea;
    else                                return Qt::TopToolBarArea;
  }

  QBoxLayout *newBoxLayout(QWidget *w, const QString &str) {
    return new QBoxLayout(stringToBoxLayoutDirection(str), w);
  }

  bool allowLayout(QWidget *w) {
    if (qobject_cast<QColorDialog *>(w) ||
        qobject_cast<QFileDialog *>(w) ||
        qobject_cast<QFontDialog *>(w) ||
        qobject_cast<QPrintDialog *>(w) ||
        qobject_cast<QProgressDialog *>(w) ||
        qobject_cast<QMainWindow *>(w) ||
        qobject_cast<QMenu *>(w))
      return false;

    return true;
  }

  QLayout *createLayout(QWidget *parent, LayoutType type, const QString &dir) {
    if      (type == HBoxLayout) return new QHBoxLayout(parent);
    else if (type == VBoxLayout) return new QVBoxLayout(parent);
    else if (type == BoxLayout ) return newBoxLayout(parent, dir);
    else if (type == GridLayout) return new QGridLayout(parent);
    else if (type == FormLayout) return new QFormLayout(parent);
    else if (type == NoLayout  ) return nullptr;
    else                         assert(false);
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

  CXMLTag *createTag(const CXML *tag, CXMLTag *parent, const std::string &name,
                     CXMLTag::OptionArray &options) override;

  void createWidgets(QWidget *parent);

  void createWidgets(CXMLTag *tag, QLayout *layout);
  void createWidgets(CXMLTag *tag, QWidget *widget);

 private:
  CQXml        *xml_;
  CQXmlRootTag *root_;
};

class CQXmlTag : public CXMLTag {
 public:
  CQXmlTag(const CXML *xml, CXMLTag *parent, const std::string &name,
           CXMLTag::OptionArray &options) :
   CXMLTag(xml, parent, name, options) {
  }

  virtual ~CQXmlTag() { }

  CQXml *getXml() const;

  virtual bool isRoot  () const { return false; }
  virtual bool isLayout() const { return false; }
  virtual bool isWidget() const { return false; }
  virtual bool isExec  () const { return false; }

  virtual QLayout *createLayout(QWidget *, QLayout *, CQXmlTag *) { return nullptr; }

  virtual QWidget *createLayoutChild(QLayout *, CQXmlTag *) { return nullptr; }
  virtual QWidget *createWidgetChild(QWidget *, CQXmlTag *) { return nullptr; }

  virtual bool exec(QWidget *, QLayout *) { return false; }

  virtual void endLayout() { }

  virtual void handleOptions(CXMLTag::OptionArray &options) {
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
  typedef std::map<QString, QString> NameValues;

  NameValues nameValues_;
};

class CQXmlLayoutTag : public CQXmlTag {
 public:
  CQXmlLayoutTag(const CXML *xml, CXMLTag *parent, CQXmlUtil::LayoutType type,
                 const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options), type_(type) {
  }

  bool isLayout() const override { return true; }

  QLayout *createLayout(QWidget *w, QLayout *l, CQXmlTag *) override {
    layout_ = CQXmlUtil::createLayout(w, type_, nameValue("direction"));

    layout_->setMargin(0); layout_->setSpacing(0);

    int margin = 2, spacing = 2;

    if (hasNameValue("margin" )) margin  = nameValue("margin" ).toInt();
    if (hasNameValue("spacing")) spacing = nameValue("spacing").toInt();

    layout_->setMargin(margin); layout_->setSpacing(spacing);

    if (l) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout  *>(l)->addLayout(layout_);
      else if (qobject_cast<QGridLayout *>(l))
        qobject_cast<QGridLayout *>(l)->addLayout(layout_, 0, 0);
      else if (qobject_cast<QFormLayout *>(l)) {
        auto label = nameValue("formLabel");

        qobject_cast<QFormLayout *>(l)->addRow(label, layout_);
      }
      else
        assert(false);
    }

    if (hasNameValue("name"))
      getXml()->addLayout(nameValue("name"), layout_);

    return layout_;
  }

  bool handleOption(const std::string &name, const std::string &value) override {
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

  void endLayout() override {
    if      (qobject_cast<QBoxLayout *>(layout_)) {
    }
    else if (qobject_cast<QGridLayout *>(layout_)) {
      auto *grid = qobject_cast<QGridLayout *>(layout_);

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
  typedef std::pair<int, int>      IntIntPair;
  typedef std::vector<IntIntPair> IntIntPairArray;

  CQXmlUtil::LayoutType type_;
  QLayout*              layout_;
  IntIntPairArray       columnStretches_;
  IntIntPairArray       rowStretches_;
};

class CQXmlRootTag : public CQXmlTag {
 public:
  CQXmlRootTag(CXMLTag *parent, CQXml *xml, const std::string &name,
               CXMLTag::OptionArray &options) :
   CQXmlTag(xml->getXml(), parent, name, options), xml_(xml), type_(CQXmlUtil::VBoxLayout) {
    auto *factory = xml->getFactory();

    factory->setRoot(this);

    for (auto option : options) {
      const std::string &name  = option->getName();
      const std::string &value = option->getValue();

      if (name == "layoutType")
        type_ = stringToLayoutType(value.c_str());
    }
  }

  bool isRoot() const override { return true; }

  CQXml *getXml() const { return xml_; }

  bool handleOption(const std::string &name, const std::string &value) override {
    if (name == "windowTitle")
      getXml()->parent()->setWindowTitle(value.c_str());
    else
      return false;

    return true;
  }

  QLayout *createRootLayout(QWidget *parent) {
    return CQXmlUtil::createLayout(parent, type_, nameValue("direction"));
  }

 private:
  CQXml                 *xml_;
  CQXmlUtil::LayoutType  type_;
};

class CQXmlStyleTag : public CQXmlTag {
 public:
  CQXmlStyleTag(const CXML *xml, CXMLTag *parent, const std::string &style,
                const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options), style_(style) {
  }

  bool isWidget() const override { return true; }

  QWidget *createLayoutChild(QLayout *l, CQXmlTag *) override {
    return CQStyleWidgetMgrInst->addStyleLabel(l, getText(), style_.c_str());
  }

 private:
  std::string style_;
};

class CQXmlLayoutItemTag : public CQXmlTag {
 public:
  CQXmlLayoutItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                     CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isLayout() const override { return true; }

  QLayout *createLayout(QWidget *, QLayout *l, CQXmlTag *) override {
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
  CQXmlComboItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                    CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
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
  CQXmlListItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                   CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    if (! qobject_cast<QListWidget *>(w)) return w;

    qobject_cast<QListWidget *>(w)->addItem(getText());

    return w;
  }
};

class CQXmlTableItemTag : public CQXmlTag {
 public:
  CQXmlTableItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                    CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    if (! qobject_cast<QTableWidget *>(w)) return w;

    auto *item = new QTableWidgetItem(getText());

    int row = nameValue("row"   ).toInt();
    int col = nameValue("column").toInt();

    qobject_cast<QTableWidget *>(w)->setItem(row, col, item);

    return w;
  }
};

class CQXmlTreeItemTag : public CQXmlTag {
 public:
  CQXmlTreeItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                   CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    if (! qobject_cast<QTreeWidget *>(w)) return w;

    auto items = getText().split(' ');

    auto *item = new QTreeWidgetItem(items);

    qobject_cast<QTreeWidget *>(w)->addTopLevelItem(item);

    return w;
  }
};

class CQXmlTabItemTag : public CQXmlTag {
 public:
  CQXmlTabItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                  CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
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
  CQXmlMenuTitleTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                    CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    if (! qobject_cast<QMenuBar *>(w)) return w;

    auto *menu = qobject_cast<QMenuBar *>(w)->addMenu(getText());

    return menu;
  }
};

class CQXmlActionTag : public CQXmlTag {
 public:
  CQXmlActionTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                 CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isWidget() const override { return true; }

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    QAction *action = nullptr;

    if      (hasNameValue("actionRef"))
      action = getXml()->getAction(nameValue("actionRef"));
    else if (hasNameValue("icon")) {
      QPixmap pixmap(nameValue("icon"));

      action = new QAction(QIcon(pixmap), getText(), nullptr);
    }
    else if (getText().length())
      action = new QAction(getText(), nullptr);

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
  CQXmlConnectTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                  CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isExec() const override { return true; }

  bool exec(QWidget *, QLayout *) override {
    QWidget *source = nullptr, *dest = nullptr;

    if (hasNameValue("source"))
      source = getXml()->getWidget(nameValue("source"));

    if (hasNameValue("dest"))
      dest = getXml()->getWidget(nameValue("dest"));

    auto sourceSignal = "2" + nameValue("sourceSignal");

    if (hasNameValue("destSignal")) {
      auto destSignal = "2" + nameValue("destSignal");

      QObject::connect(source, sourceSignal.toLatin1(), dest, destSignal.toLatin1());
    }
    else {
      auto destSlot = "1" + nameValue("destSlot");

      QObject::connect(source, sourceSignal.toLatin1(), dest, destSlot.toLatin1());
    }

    return false;
  }
};

class CQXmlPropertyItemTag : public CQXmlTag {
 public:
  CQXmlPropertyItemTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                       CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, name, options) {
  }

  bool isExec() const override { return true; }

  bool exec(QWidget *widget, QLayout *) override {
    if (! widget) return false;

    auto propertyPath = nameValue("propertyPath");
    auto propertyName = nameValue("propertyName");

    if (! propertyName.size())
      return false;

    auto *propertyWidget = getXml()->getWidget(nameValue("propertyWidget"));
    if (! propertyWidget) return false;

    auto *tree = qobject_cast<CQPropertyTree *>(widget);
    if (! tree) return false;

    tree->addProperty(propertyPath, propertyWidget, propertyName);

    return true;
  }
};

class CQXmlQtWidgetTag : public CQXmlTag {
 public:
  CQXmlQtWidgetTag(const CXML *xml, CXMLTag *parent, const std::string &type,
                   CXMLTag::OptionArray &options) :
   CQXmlTag(xml, parent, type, options), type_(type.c_str()) {
  }

  bool isWidget() const override { return true; }

  QWidget *createLayoutChild(QLayout *l, CQXmlTag *) override {
    auto text = getText();

    auto *w = createWidgetI(text);

    if (l && CQXmlUtil::allowLayout(w)) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout *>(l)->addWidget(w);
      else if (qobject_cast<QGridLayout *>(l)) {
        int row = nameValue("row").toInt();
        int col = nameValue("col").toInt();

        qobject_cast<QGridLayout *>(l)->addWidget(w, row, col);
      }
      else if (qobject_cast<QFormLayout *>(l)) {
        auto label = nameValue("formLabel");

        qobject_cast<QFormLayout *>(l)->addRow(label, w);
      }
    }

    if (hasNameValue("menuRef")) {
      auto *menu = qobject_cast<QMenu *>(getXml()->getWidget(nameValue("menuRef")));
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

  QWidget *createWidgetChild(QWidget *w, CQXmlTag *) override {
    auto text = getText();

    auto *w1 = createWidgetI(text);

    if      (qobject_cast<QTabWidget *>(w)) {
      auto text1 = nameValue("tabText");

      if (hasNameValue("tabIcon")) {
        QPixmap pixmap(nameValue("tabIcon"));

        qobject_cast<QTabWidget *>(w)->addTab(w1, QIcon(pixmap), text1);
      }
      else
        qobject_cast<QTabWidget *>(w)->addTab(w1, text1);
    }
    else if (qobject_cast<QToolBox *>(w)) {
      auto text1 = nameValue("toolText");

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
        auto area = Qt::LeftDockWidgetArea;

        auto dockWidgetArea = nameValue("dockWidgetArea");

        if (dockWidgetArea.length())
          area = CQXmlUtil::stringToDockWidgetArea(dockWidgetArea);

        qobject_cast<QMainWindow *>(w)->addDockWidget(area, qobject_cast<QDockWidget *>(w1));
      }
      else if (qobject_cast<QToolBar *>(w1)) {
        auto area = Qt::TopToolBarArea;

        auto toolBarArea = nameValue("toolBarArea");

        if (toolBarArea.length())
          area = CQXmlUtil::stringToToolBarArea(toolBarArea);

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
    auto *xml = getXml();

    auto *factory = xml->getWidgetFactory(type_);

    auto *w = factory->createWidget(options_);

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

    const auto *meta = w->metaObject();

    if (meta) {
      for (auto nameValue : nameValues_) {
        int propIndex = meta->indexOfProperty(nameValue.first.toLatin1());
        if (propIndex < 0) continue;

        auto mP = meta->property(propIndex);
        if (! mP.isWritable()) continue;

        if (mP.isEnumType()) {
          QString name(nameValue.second);

          auto me = mP.enumerator();

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
            if (! v.convert(int(mP.type())))
              continue;
          }

          (void) w->setProperty(nameValue.first.toLatin1(), v);
        }
      }
    }

    if      (qobject_cast<QTableWidget *>(w)) {
      auto columnLabels = nameValue("columnLabels").split(' ');

      if (columnLabels.length())
        qobject_cast<QTableWidget *>(w)->setHorizontalHeaderLabels(columnLabels);

      auto rowLabels = nameValue("rowLabels").split(' ');

      if (rowLabels.length())
        qobject_cast<QTableWidget *>(w)->setVerticalHeaderLabels(rowLabels);
    }
    else if (qobject_cast<QTreeWidget *>(w)) {
      auto columnLabels = nameValue("columnLabels").split(' ');

      if (columnLabels.length())
        qobject_cast<QTreeWidget *>(w)->setHeaderLabels(columnLabels);
    }

    if (hasNameValue("minimumSize")) {
      auto sizes = nameValue("minimumSize").split(' ');

      if (sizes.length() == 2) {
        int w1 = sizes[0].toInt();
        int w2 = sizes[1].toInt();

        w->setMinimumSize(QSize(w1, w2));
      }
    }
    if (hasNameValue("minimumWidth")) {
      int w1 = nameValue("minimumWidth").toInt();

      w->setMinimumWidth(w1);
    }
    if (hasNameValue("maximumHeight")) {
      int h1 = nameValue("maximumHeight").toInt();

      w->setMinimumHeight(h1);
    }
    if (hasNameValue("maximumSize")) {
      auto sizes = nameValue("maximumSize").split(' ');

      if (sizes.length() == 2) {
        int h1 = sizes[0].toInt();
        int h2 = sizes[1].toInt();

        w->setMaximumSize(QSize(h1, h2));
      }
    }
    if (hasNameValue("maximumWidth")) {
      int w1 = nameValue("manimumWidth").toInt();

      w->setMaximumWidth(w1);
    }
    if (hasNameValue("maximumHeight")) {
      int h1 = nameValue("manimumHeight").toInt();

      w->setMaximumHeight(h1);
    }
    if (hasNameValue("fixedSize")) {
      auto sizes = nameValue("fixedSize").split(' ');

      if (sizes.length() == 2) {
        int w1 = sizes[0].toInt();
        int h1 = sizes[1].toInt();

        w->setMinimumWidth (w1); w->setMaximumWidth (w1);
        w->setMinimumHeight(h1); w->setMaximumHeight(h1);
      }
    }
    if (hasNameValue("fixedWidth")) {
      int w1 = nameValue("fixedWidth").toInt();

      w->setMinimumWidth(w1); w->setMaximumWidth(w1);
    }
    if (hasNameValue("fixedHeight")) {
      int h1 = nameValue("fixedHeight").toInt();

      w->setMinimumHeight(h1); w->setMaximumHeight(h1);
    }

    return w;
  }

 private:
  QString     type_;
  QStringList options_;
};

class CQXmlLayoutTagFactory : public CQXmlTagFactory {
 public:
  CQXmlLayoutTagFactory(CQXmlUtil::LayoutType type) :
   type_(type) {
  }

  CQXmlTag *createTag(const CXML *tag, CXMLTag *parent, const std::string &name,
                      CXMLTag::OptionArray &options) override {
    return new CQXmlLayoutTag(tag, parent, type_, name, options);
  }

 private:
  CQXmlUtil::LayoutType type_;
};

class CQXmlStyleTagFactory : public CQXmlTagFactory {
 public:
  CQXmlStyleTagFactory(const std::string &style) :
   style_(style) {
  }

  CQXmlTag *createTag(const CXML *tag, CXMLTag *parent, const std::string &name,
                      CXMLTag::OptionArray &options) override {
    return new CQXmlStyleTag(tag, parent, style_, name, options);
  }

 private:
  std::string style_;
};

//------

CQXml::
CQXml() :
 parent_(nullptr)
{
  xml_ = new CXML;

  factory_ = new CQXmlFactory(this);

  xml_->setFactory(factory_);

  CQXmlAddWidgetFactoryT(this, QCalendarWidget);
  CQXmlAddWidgetFactoryT(this, QCheckBox);
  CQXmlAddWidgetFactoryT(this, QColorDialog);
  CQXmlAddWidgetFactoryT(this, QComboBox);
  CQXmlAddWidgetFactoryT(this, QDial);
  CQXmlAddWidgetFactoryT(this, QDialog);
  CQXmlAddWidgetFactoryT(this, QDateEdit);
  CQXmlAddWidgetFactoryT(this, QDateTimeEdit);
  CQXmlAddWidgetFactoryT(this, QDockWidget);
  CQXmlAddWidgetFactoryT(this, QDoubleSpinBox);
  CQXmlAddWidgetFactoryT(this, QFileDialog);
  CQXmlAddWidgetFactoryT(this, QFontComboBox);
  CQXmlAddWidgetFactoryT(this, QFontDialog);
  CQXmlAddWidgetFactoryT(this, QFrame);
  CQXmlAddWidgetFactoryT(this, QGroupBox);
  CQXmlAddWidgetFactoryT(this, QLabel);
  CQXmlAddWidgetFactoryT(this, QLCDNumber);
  CQXmlAddWidgetFactoryT(this, QLineEdit);
  CQXmlAddWidgetFactoryT(this, QListView);
  CQXmlAddWidgetFactoryT(this, QListWidget);
  CQXmlAddWidgetFactoryT(this, QMainWindow);
  CQXmlAddWidgetFactoryT(this, QMdiArea);
  CQXmlAddWidgetFactoryT(this, QMdiSubWindow);
  CQXmlAddWidgetFactoryT(this, QMenu);
  CQXmlAddWidgetFactoryT(this, QMenuBar);
  CQXmlAddWidgetFactoryT(this, QMessageBox);
  CQXmlAddWidgetFactoryT(this, QPlainTextEdit);
  CQXmlAddWidgetFactoryT(this, QPrintDialog);
  CQXmlAddWidgetFactoryT(this, QProgressBar);
  CQXmlAddWidgetFactoryT(this, QProgressDialog);
  CQXmlAddWidgetFactoryT(this, QPushButton);
  CQXmlAddWidgetFactoryT(this, QRadioButton);
  CQXmlAddWidgetFactoryT(this, QScrollArea);
  CQXmlAddWidgetFactoryT(this, QScrollBar);
  CQXmlAddWidgetFactoryT(this, QSlider);
  CQXmlAddWidgetFactoryT(this, QSpinBox);
  CQXmlAddWidgetFactoryT(this, QSplitter);
  CQXmlAddWidgetFactoryT(this, QStackedWidget);
  CQXmlAddWidgetFactoryT(this, QStatusBar);
  CQXmlAddWidgetFactoryT(this, QTabBar);
  CQXmlAddWidgetFactoryT(this, QTabWidget);
  CQXmlAddWidgetFactoryT(this, QTableView);
  CQXmlAddWidgetFactoryT(this, QTableWidget);
  CQXmlAddWidgetFactoryT(this, QTextEdit);
  CQXmlAddWidgetFactoryT(this, QToolBar);
  CQXmlAddWidgetFactoryT(this, QToolBox);
  CQXmlAddWidgetFactoryT(this, QToolButton);
  CQXmlAddWidgetFactoryT(this, QTreeView);
  CQXmlAddWidgetFactoryT(this, QTreeWidget);
  CQXmlAddWidgetFactoryT(this, QUndoView);
  CQXmlAddWidgetFactoryT(this, QWidget);
  CQXmlAddWidgetFactoryT(this, QWebView);
  CQXmlAddWidgetFactoryT(this, QWizard);
  CQXmlAddWidgetFactoryT(this, QWizardPage);

  CQXmlAddWidgetFactoryT(this, CQPropertyTree);
  CQXmlAddTagFactoryT(this, "CQPropertyItem", CQXmlPropertyItemTag);

  CQXmlAddTagFactoryT(this, "QLayoutItem", CQXmlLayoutItemTag);
  CQXmlAddTagFactoryT(this, "QComboItem" , CQXmlComboItemTag );
  CQXmlAddTagFactoryT(this, "QListItem"  , CQXmlListItemTag  );
  CQXmlAddTagFactoryT(this, "QTableItem" , CQXmlTableItemTag );
  CQXmlAddTagFactoryT(this, "QTreeItem"  , CQXmlTreeItemTag  );
  CQXmlAddTagFactoryT(this, "QTabItem"   , CQXmlTabItemTag   );
  CQXmlAddTagFactoryT(this, "QMenuTitle" , CQXmlMenuTitleTag );
  CQXmlAddTagFactoryT(this, "QAction"    , CQXmlActionTag    );

  CQXmlAddTagFactoryT(this, "connect", CQXmlConnectTag);

  addTagFactory("QHBoxLayout", new CQXmlLayoutTagFactory(CQXmlUtil::HBoxLayout));
  addTagFactory("QVBoxLayout", new CQXmlLayoutTagFactory(CQXmlUtil::VBoxLayout));
  addTagFactory("QBoxLayout" , new CQXmlLayoutTagFactory(CQXmlUtil::BoxLayout ));
  addTagFactory("QGridLayout", new CQXmlLayoutTagFactory(CQXmlUtil::GridLayout));
  addTagFactory("QFormLayout", new CQXmlLayoutTagFactory(CQXmlUtil::FormLayout));

  addTagFactory("h1", new CQXmlStyleTagFactory("h1"));
  addTagFactory("h2", new CQXmlStyleTagFactory("h2"));
  addTagFactory("h3", new CQXmlStyleTagFactory("h3"));
  addTagFactory("h4", new CQXmlStyleTagFactory("h4"));
  addTagFactory("p" , new CQXmlStyleTagFactory("p" ));
}

CQXml::
~CQXml()
{
  delete xml_;
}

//-----

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

//------

bool
CQXml::
isTagFactory(const QString &name) const
{
  return (tagFactories_.find(name) != tagFactories_.end());
}

void
CQXml::
addTagFactory(const QString &name, CQXmlTagFactory *factory)
{
  if (isTagFactory(name))
    removeTagFactory(name);

  tagFactories_[name] = factory;
}

void
CQXml::
removeTagFactory(const QString &name)
{
  TagFactories::iterator p = tagFactories_.find(name);
  assert(p != tagFactories_.end());

  tagFactories_.erase(p);
}

CQXmlTagFactory *
CQXml::
getTagFactory(const QString &name) const
{
  TagFactories::const_iterator p = tagFactories_.find(name);
  assert(p != tagFactories_.end());

  return (*p).second;
}

//------

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
    return nullptr;

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
    return nullptr;

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
    return nullptr;

  return (*p).second;
}

//-------

CXMLTag *
CQXmlFactory::
createTag(const CXML *xml, CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options)
{
  typedef std::map<QString, QString> NameValues;

  NameValues nameValues;

  for (auto option : options) {
    const std::string &name  = option->getName();
    const std::string &value = option->getValue();

    nameValues[name.c_str()] = value.c_str();
  }

  //---

  CQXmlTag *tag = nullptr;

  if      (name == "qxml")
    tag = new CQXmlRootTag(parent, xml_, name, options);
  else if (xml_->isTagFactory(name.c_str()))
    tag = xml_->getTagFactory(name.c_str())->createTag(xml, parent, name, options);
  else if (xml_->isWidgetFactory(name.c_str()))
    tag = new CQXmlQtWidgetTag(xml, parent, name, options);
  else {
    std::cerr << "Invalid tag name " << name << std::endl;
    return CXMLFactory::createTag(xml, parent, name, options);
  }

  if (tag)
    tag->handleOptions(options);

  return tag;
}

void
CQXmlFactory::
createWidgets(QWidget *parent)
{
  QLayout *layout = nullptr;

  if (CQXmlUtil::allowLayout(parent))
    layout = root_->createRootLayout(parent);

  if (layout)
    createWidgets(root_, layout);
  else
    createWidgets(root_, parent);
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QLayout *layout)
{
  auto *ptag = dynamic_cast<CQXmlTag *>(tag);

  for (size_t i = 0; i < tag->getNumChildren(); ++i) {
    const auto *token = tag->getChild(int(i));

    if (token->isTag()) {
      auto *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if      (tag1->isLayout()) {
        auto *layout1 = tag1->createLayout(nullptr, layout, ptag);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else if (tag1->isWidget()) {
        auto *widget = tag1->createLayoutChild(layout, ptag);

        createWidgets(tag1, widget);
      }
      else if (tag1->isExec()) {
        (void) tag1->exec(nullptr, layout);
      }
    }
  }
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QWidget *widget)
{
  auto *ptag = dynamic_cast<CQXmlTag *>(tag);

  for (size_t i = 0; i < tag->getNumChildren(); ++i) {
    const auto *token = tag->getChild(int(i));

    if (token->isTag()) {
      auto *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if      (tag1->isLayout()) {
        auto *layout1 = tag1->createLayout(widget, nullptr, ptag);

        if (qobject_cast<QGroupBox *>(widget))
          qobject_cast<QGroupBox *>(widget)->setLayout(layout1);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else if (tag1->isWidget()) {
        auto *widget1 = tag1->createWidgetChild(widget, ptag);

        createWidgets(tag1, widget1);
      }
      else if (tag1->isExec()) {
        (void) tag1->exec(widget, nullptr);
      }
    }
  }
}

//------

CQXml *
CQXmlTag::
getXml() const
{
  auto *parent = dynamic_cast<CQXmlTag *>(getParent());

  while (parent && ! parent->isRoot())
    parent = dynamic_cast<CQXmlTag *>(parent->getParent());

  if (parent)
    return dynamic_cast<CQXmlRootTag *>(parent)->getXml();

  return nullptr;
}
