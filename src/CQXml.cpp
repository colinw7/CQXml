#include <CQXml.h>
#include <CQStyleWidget.h>
#include <CXML.h>
#include <CXMLToken.h>
#include <CStrUtil.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMetaProperty>

#include <iostream>
#include <cassert>

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

  virtual QWidget *createWidget(QLayout *, const QString &) { return 0; }
  virtual QLayout *createLayout(QLayout *) { return 0; }

  virtual void endLayout() { }

  void handleOptions(CXMLTag::OptionArray &options) {
    CXMLTag::OptionArray::const_iterator o1, o2;

    for (auto option : options) {
      if (! handleOption(option->getName(), option->getValue()))
        std::cerr << "Unhandled option: " << option->getName() << "=" <<
                     option->getValue() << std::endl;
    }
  }

  virtual bool handleOption(const std::string &, const std::string &) { return false; }
};

class CQXmlRootTag : public CQXmlTag {
 public:
  CQXmlRootTag(CXMLTag *parent, CQXml *xml, const std::string &name,
               CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), xml_(xml) {
    CQXmlFactory *factory = xml->getFactory();

    factory->setRoot(this);
  }

  bool isRoot() const { return true; }

  CQXml *getXml() const { return xml_; }

 private:
  CQXml *xml_;
};

class CQXmlHeadingTag : public CQXmlTag {
 public:
  CQXmlHeadingTag(CXMLTag *parent, int level, const std::string &name,
                  CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), level_(level) {
  }

  QWidget *createWidget(QLayout *l, const QString &text) {
    QString type = QString("h%1").arg(level_);

    return CQStyleWidgetMgrInst->addStyleLabel(l, text, type);
  }

 private:
  int level_;
};

class CQXmlPTag : public CQXmlTag {
 public:
  CQXmlPTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options) {
  }

  QWidget *createWidget(QLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addParagraph(l, text);
  }
};

class CQXmlLayoutTag : public CQXmlTag {
 public:
  enum Type  {
   HBox,
   VBox,
   Grid
  };

 public:
  CQXmlLayoutTag(CXMLTag *parent, Type type, const std::string &name,
                 CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), type_(type), margin_(2), spacing_(2) {
  }

  QLayout *createLayout(QLayout *l) {
    if      (type_ == HBox) layout_ = new QHBoxLayout;
    else if (type_ == VBox) layout_ = new QVBoxLayout;
    else if (type_ == Grid) layout_ = new QGridLayout;
    else                    assert(false);

    layout_->setMargin (margin_ );
    layout_->setSpacing(spacing_);

    if (l) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout *>(l)->addLayout(layout_);
      else if (qobject_cast<QGridLayout *>(l))
        qobject_cast<QGridLayout *>(l)->addLayout(layout_, 0, 0);
      else
        assert(false);
    }

    getXml()->addLayout(name_, layout_);

    return layout_;
  }

  bool isLayout() const { return true; }

  bool handleOption(const std::string &name, const std::string &value) {
    if      (name == "name")
      name_ = value.c_str();
    else if (name == "columnStretch") {
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
    else if (name == "margin")
      margin_  = std::stoi(value);
    else if (name == "spacing")
      spacing_ = std::stoi(value);
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
    else
      assert(false);
  }

 private:
  typedef std::pair<int,int>      IntIntPair;
  typedef std::vector<IntIntPair> IntIntPairArray;

  Type            type_;
  QString         name_;
  QLayout*        layout_;
  int             margin_, spacing_;
  IntIntPairArray columnStretches_;
  IntIntPairArray rowStretches_;
};

class CQXmlLayoutItemTag : public CQXmlTag {
 public:
  CQXmlLayoutItemTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, name, options), spacing_(0), spacingSet_(false),
   stretch_(0), stretchSet_(false) {
  }

  QLayout *createLayout(QLayout *l) {
    if      (qobject_cast<QBoxLayout *>(l)) {
      if (spacingSet_)
        qobject_cast<QBoxLayout *>(l)->addSpacing(spacing_);

      if (stretchSet_)
        qobject_cast<QBoxLayout *>(l)->addStretch(stretch_);
    }

    return l;
  }

  bool isLayout() const { return true; }

  bool handleOption(const std::string &name, const std::string &value) {
    if      (name == "spacing") {
      spacing_ = std::stoi(value.c_str()); spacingSet_ = true;
    }
    else if (name == "stretch") {
      stretch_ = std::stoi(value.c_str()); stretchSet_ = true;
    }
    else
      return false;

    return true;
  }

 private:
  int  spacing_;
  bool spacingSet_;
  int  stretch_;
  bool stretchSet_;
};

class CQXmlQtWidgetTag : public CQXmlTag {
 public:
  CQXmlQtWidgetTag(CXMLTag *parent, const std::string &type, CXMLTag::OptionArray &options) :
   CQXmlTag(parent, type, options), type_(type.c_str()), row_(0), col_(0) {
  }

  QWidget *createWidget(QLayout *l, const QString &text) {
    CQXml *xml = getXml();

    CQXmlWidgetFactory *factory = xml->getWidgetFactory(type_);

    QWidget *w = factory->createWidget(options_);

    if (name_.length()) {
      w->setObjectName(name_);

      getXml()->addWidget(name_, w);
    }

    if      (qobject_cast<QLabel *>(w))
      qobject_cast<QLabel *>(w)->setText(text);
    else if (qobject_cast<QAbstractButton *>(w))
      qobject_cast<QAbstractButton *>(w)->setText(text);

    const QMetaObject *meta = w->metaObject();

    if (meta) {
      for (auto nameValue : nameValues_) {
        int propIndex = meta->indexOfProperty(nameValue.first.c_str());
        if (propIndex < 0) continue;

        QMetaProperty mP = meta->property(propIndex);
        if (! mP.isWritable()) continue;

        if (mP.isEnumType()) {
          QString name(nameValue.second.c_str());

          QMetaEnum me = mP.enumerator();

          for (int i = 0; i < me.keyCount(); ++i) {
            if (me.key(i) == name)
              (void) w->setProperty(nameValue.first.c_str(), me.value(i));
          }
        }
        else {
          QVariant v(nameValue.second.c_str());

          if (mP.type() == QVariant::Icon) {
            QPixmap pixmap(nameValue.second.c_str());

            v = QIcon(pixmap);
          }
          else {
            if (! v.convert(mP.type()))
              continue;
          }

          (void) w->setProperty(nameValue.first.c_str(), v);
        }
      }
    }

    if (l) {
      if      (qobject_cast<QBoxLayout *>(l))
        qobject_cast<QBoxLayout *>(l)->addWidget(w);
      else if (qobject_cast<QGridLayout *>(l))
        qobject_cast<QGridLayout *>(l)->addWidget(w, row_, col_);
    }

    return w;
  }

  bool handleOption(const std::string &name, const std::string &value) {
    if      (name == "name") name_ = value.c_str();
    else if (name == "row" ) row_  = std::stoi(value);
    else if (name == "col" ) col_  = std::stoi(value);
    else                     nameValues_[name] = value;

    return true;
  }

 private:
  typedef std::map<std::string,std::string> NameValues;

  QString     type_;
  QString     name_;
  int         row_, col_;
  QStringList options_;
  NameValues  nameValues_;
};

//------

CQXml::
CQXml()
{
  xml_ = new CXML;

  factory_ = new CQXmlFactory(this);

  xml_->setFactory(factory_);

  addWidgetFactory("QCheckBox"     , new QtWidgetFactory<QCheckBox>     ());
  addWidgetFactory("QComboBox"     , new QtWidgetFactory<QComboBox>     ());
  addWidgetFactory("QDoubleSpinBox", new QtWidgetFactory<QDoubleSpinBox>());
  addWidgetFactory("QFrame"        , new QtWidgetFactory<QFrame>        ());
  addWidgetFactory("QGroupBox"     , new QtWidgetFactory<QGroupBox>     ());
  addWidgetFactory("QLabel"        , new QtWidgetFactory<QLabel>        ());
  addWidgetFactory("QLineEdit"     , new QtWidgetFactory<QLineEdit>     ());
  addWidgetFactory("QListWidget"   , new QtWidgetFactory<QListWidget>   ());
  addWidgetFactory("QListView"     , new QtWidgetFactory<QListView>     ());
  addWidgetFactory("QPushButton"   , new QtWidgetFactory<QPushButton>   ());
  addWidgetFactory("QRadioButton"  , new QtWidgetFactory<QRadioButton>  ());
  addWidgetFactory("QScrollBar"    , new QtWidgetFactory<QScrollBar>    ());
  addWidgetFactory("QSlider"       , new QtWidgetFactory<QSlider>       ());
  addWidgetFactory("QSpinBox"      , new QtWidgetFactory<QSpinBox>      ());
  addWidgetFactory("QSplitter"     , new QtWidgetFactory<QSplitter>     ());
  addWidgetFactory("QStackedWidget", new QtWidgetFactory<QStackedWidget>());
  addWidgetFactory("QTextEdit"     , new QtWidgetFactory<QTextEdit>     ());
  addWidgetFactory("QToolButton"   , new QtWidgetFactory<QToolButton>   ());
  addWidgetFactory("QTreeWidget"   , new QtWidgetFactory<QTreeWidget>   ());
  addWidgetFactory("QTreeView"     , new QtWidgetFactory<QTreeView>     ());
  addWidgetFactory("QWidget"       , new QtWidgetFactory<QWidget>       ());
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
  CXMLTag *tag;

  if (! xml_->readString(str, &tag))
    return;

  factory_->createWidgets(parent);
}

void
CQXml::
createWidgetsFromFile(QWidget *parent, const std::string &filename)
{
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

//-------

CXMLTag *
CQXmlFactory::
createTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options)
{
  CQXmlTag *tag = 0;

  if      (name == "qxml")
    tag = new CQXmlRootTag(parent, xml_, name, options);
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
  else if (name == "QHBoxLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::HBox, name, options);
  else if (name == "QVBoxLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::VBox, name, options);
  else if (name == "QGridLayout")
    tag = new CQXmlLayoutTag(parent, CQXmlLayoutTag::Grid, name, options);
  else if (name == "QLayoutItem")
    tag = new CQXmlLayoutItemTag(parent, name, options);
  else if (xml_->isWidgetFactory(name.c_str()))
    tag = new CQXmlQtWidgetTag(parent, name, options);
  else {
    std::cerr << "Invalid tag name " << name << std::endl;
    return CXMLFactory::createTag(parent, name, options);
  }

  CQXmlTag *tag1 = dynamic_cast<CQXmlTag *>(tag);

  if (tag1)
    tag1->handleOptions(options);

  return tag;
}

void
CQXmlFactory::
createWidgets(QWidget *parent)
{
  QVBoxLayout *layout = new QVBoxLayout(parent);
  layout->setMargin(0); layout->setSpacing(0);

  createWidgets(root_, layout);

  layout->addStretch();
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QLayout *layout)
{
  for (int i = 0; i < tag->getNumChildren(); ++i) {
    const CXMLToken *token = tag->getChild(i);

    if (token->isTag()) {
      CQXmlTag *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if (tag1->isLayout()) {
        QLayout *layout1 = tag1->createLayout(layout);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else {
        std::string text = tag1->getText();

        QWidget *widget = tag1->createWidget(layout, text.c_str());

        createWidgets(tag1, widget);
      }
    }
  }
}

void
CQXmlFactory::
createWidgets(CXMLTag *tag, QWidget *widget)
{
  for (int i = 0; i < tag->getNumChildren(); ++i) {
    const CXMLToken *token = tag->getChild(i);

    if (token->isTag()) {
      CQXmlTag *tag1 = dynamic_cast<CQXmlTag *>(token->getTag());
      if (! tag1) continue;

      if (tag1->isLayout()) {
        QLayout *layout1 = tag1->createLayout(0);

        if (qobject_cast<QGroupBox *>(widget))
          qobject_cast<QGroupBox *>(widget)->setLayout(layout1);

        createWidgets(tag1, layout1);

        tag1->endLayout();
      }
      else {
        std::string text = tag1->getText();

        QWidget *widget = tag1->createWidget(0, text.c_str());

        createWidgets(tag1, widget);
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
