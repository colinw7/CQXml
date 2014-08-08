#include <CQXml.h>
#include <CQStyleWidget.h>
#include <CXML.h>
#include <CXMLToken.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
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

  void createWidgets(CXMLTag *tag, QBoxLayout *layout);

 private:
  CQXml        *xml_;
  CQXmlRootTag *root_;
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

  virtual QWidget    *createWidget(QBoxLayout *, const QString &) { return 0; }
  virtual QBoxLayout *createLayout(QBoxLayout *) { return 0; }
};

class CQXmlRootTag : public CQXmlTag {
 public:
  CQXmlRootTag(CXMLTag *parent, CQXml *xml, const std::string &name, CXMLTag::OptionArray &options);

  bool isRoot() const { return true; }

  CQXml *getXml() const { return xml_; }

 private:
  CQXml *xml_;
};

class CQXmlH1Tag : public CQXmlTag {
 public:
  CQXmlH1Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QWidget *createWidget(QBoxLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addHeader1(l, text);
  }
};

class CQXmlH2Tag : public CQXmlTag {
 public:
  CQXmlH2Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QWidget *createWidget(QBoxLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addHeader2(l, text);
  }
};

class CQXmlH3Tag : public CQXmlTag {
 public:
  CQXmlH3Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QWidget *createWidget(QBoxLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addHeader3(l, text);
  }
};

class CQXmlH4Tag : public CQXmlTag {
 public:
  CQXmlH4Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QWidget *createWidget(QBoxLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addHeader4(l, text);
  }
};

class CQXmlHLayoutTag : public CQXmlTag {
 public:
  CQXmlHLayoutTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QBoxLayout *createLayout(QBoxLayout *l) {
    QHBoxLayout *l1 = new QHBoxLayout;
    l1->setMargin(0); l1->setSpacing(2);

    l->addLayout(l1);

    getXml()->addLayout(name_.c_str(), l1);

    return l1;
  }

  bool isLayout() const { return true; }

 private:
  std::string name_;
};

class CQXmlVLayoutTag : public CQXmlTag {
 public:
  CQXmlVLayoutTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QBoxLayout *createLayout(QBoxLayout *l) {
    QVBoxLayout *l1 = new QVBoxLayout;
    l1->setMargin(0); l1->setSpacing(2);

    l->addLayout(l1);

    getXml()->addLayout(name_.c_str(), l1);

    return l1;
  }

  bool isLayout() const { return true; }

 private:
  std::string name_;
};

class CQXmlPTag : public CQXmlTag {
 public:
  CQXmlPTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options);

  QWidget *createWidget(QBoxLayout *l, const QString &text) {
    return CQStyleWidgetMgrInst->addParagraph(l, text);
  }
};

//------

CQXml::
CQXml()
{
  xml_ = new CXML;

  factory_ = new CQXmlFactory(this);

  xml_->setFactory(factory_);
}

CQXml::
~CQXml()
{
  delete xml_;
}

void
CQXml::
createWidgets(QWidget *parent, const std::string &str)
{
  CXMLTag *tag;

  if (! xml_->readString(str, &tag))
    return;

  factory_->createWidgets(parent);
}

void
CQXml::
addLayout(const QString &name, QBoxLayout *l)
{
  layouts_[name] = l;
}

QBoxLayout *
CQXml::
getLayout(const QString &name) const
{
  LayoutMap::const_iterator p = layouts_.find(name);

  if (p == layouts_.end())
    return 0;

  return (*p).second;
}

//-------

CXMLTag *
CQXmlFactory::
createTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options)
{
  if      (name == "qxml") {
    CQXmlRootTag *root = new CQXmlRootTag(parent, xml_, name, options);

    return root;
  }
  else if (name == "h1") {
    CQXmlH1Tag *h1 = new CQXmlH1Tag(parent, name, options);

    return h1;
  }
  else if (name == "h2") {
    CQXmlH2Tag *h2 = new CQXmlH2Tag(parent, name, options);

    return h2;
  }
  else if (name == "h3") {
    CQXmlH3Tag *h3 = new CQXmlH3Tag(parent, name, options);

    return h3;
  }
  else if (name == "h4") {
    CQXmlH4Tag *h4 = new CQXmlH4Tag(parent, name, options);

    return h4;
  }
  else if (name == "hlayout") {
    CQXmlHLayoutTag *hlayout = new CQXmlHLayoutTag(parent, name, options);

    return hlayout;
  }
  else if (name == "vlayout") {
    CQXmlVLayoutTag *vlayout = new CQXmlVLayoutTag(parent, name, options);

    return vlayout;
  }
  else if (name == "p") {
    CQXmlPTag *p = new CQXmlPTag(parent, name, options);

    return p;
  }
  else
    return CXMLFactory::createTag(parent, name, options);
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
createWidgets(CXMLTag *tag, QBoxLayout *layout)
{
  for (int i = 0; i < tag->getNumChildren(); ++i) {
    const CXMLToken *token = tag->getChild(i);

    if (token->isTag()) {
      CQXmlTag *tag1 = static_cast<CQXmlTag *>(token->getTag());

      if (tag1->isLayout()) {
        QBoxLayout *layout1 = tag1->createLayout(layout);

        (void) createWidgets(tag1, layout1);
      }
      else {
        std::string text = tag1->getText();

        (void) tag1->createWidget(layout, text.c_str());
      }
    }
  }
}

//------

CQXml *
CQXmlTag::
getXml() const
{
  CQXmlTag *parent = static_cast<CQXmlTag *>(getParent());

  while (parent && ! parent->isRoot())
    parent = static_cast<CQXmlTag *>(parent->getParent());

  if (parent)
    return static_cast<CQXmlRootTag *>(parent)->getXml();

  return 0;
}

CQXmlRootTag::
CQXmlRootTag(CXMLTag *parent, CQXml *xml, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options), xml_(xml)
{
  CQXmlFactory *factory = xml->getFactory();

  factory->setRoot(this);

  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlH1Tag::
CQXmlH1Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlH2Tag::
CQXmlH2Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlH3Tag::
CQXmlH3Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlH4Tag::
CQXmlH4Tag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlHLayoutTag::
CQXmlHLayoutTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    if      (option->getName() == "name") {
      name_ = option->getValue();
    }
    else
      std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlVLayoutTag::
CQXmlVLayoutTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
  CXMLTag::OptionArray::const_iterator o1, o2;

  for (o1 = options.begin(), o2 = options.end(); o1 != o2; ++o1) {
    const CXMLTagOption *option = *o1;

    if      (option->getName() == "name") {
      name_ = option->getValue();
    }
    else
      std::cerr << "Unhandled option: " << option->getValue() << std::endl;
  }
}

CQXmlPTag::
CQXmlPTag(CXMLTag *parent, const std::string &name, CXMLTag::OptionArray &options) :
 CQXmlTag(parent, name, options)
{
}
