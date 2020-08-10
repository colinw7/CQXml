#ifndef CQXml_H
#define CQXml_H

#include <string>
#include <map>

#include <QObject>
#include <QString>
#include <QStringList>

#include <CXML.h>
#include <CXMLTag.h>

class CQXmlTag;
class CQXmlFactory;

class QWidget;
class QLayout;
class QAction;

//----

typedef std::map<QString,QString> CQXmlNameValues;

//----

class CQXmlWidgetFactory {
 public:
  CQXmlWidgetFactory() { }

  virtual ~CQXmlWidgetFactory() { }

  virtual QWidget *createWidget(const QStringList &params=QStringList()) = 0;
};

//----

template<typename T>
class CQXmlWidgetFactoryT : public CQXmlWidgetFactory {
 public:
  CQXmlWidgetFactoryT() { }

  QWidget *createWidget(const QStringList &) {
    return new T;
  }
};

#define CQXmlAddWidgetFactoryT(XML, N) \
(XML)->addWidgetFactory(#N, new CQXmlWidgetFactoryT<N>())

//----

class CQXmlTagFactory {
 public:
  CQXmlTagFactory() { }

  virtual ~CQXmlTagFactory() { }

  virtual CQXmlTag *createTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                              CXMLTag::OptionArray &options) = 0;
};

//----

template<typename T>
class CQXmlTagFactoryT : public CQXmlTagFactory {
 public:
  CQXmlTagFactoryT() { }

  CQXmlTag *createTag(const CXML *xml, CXMLTag *parent, const std::string &name,
                      CXMLTag::OptionArray &options) {
    return new T(xml, parent, name, options);
  }
};

#define CQXmlAddTagFactoryT(XML, N, T) \
(XML)->addTagFactory(N, new CQXmlTagFactoryT<T>())

//----

class CQXml {
 public:
  CQXml();
 ~CQXml();

  CXML *getXml() const { return xml_; }

  QWidget *parent() const { return parent_; }

  CQXmlFactory *getFactory() const { return factory_; }

  bool isWidgetFactory(const QString &name) const;
  void addWidgetFactory(const QString &name, CQXmlWidgetFactory *factory);
  void removeWidgetFactory(const QString &name);
  CQXmlWidgetFactory *getWidgetFactory(const QString &name) const;

  bool isTagFactory(const QString &name) const;
  void addTagFactory(const QString &name, CQXmlTagFactory *factory);
  void removeTagFactory(const QString &name);
  CQXmlTagFactory *getTagFactory(const QString &name) const;

  void createWidgetsFromString(QWidget *parent, const std::string &str);
  void createWidgetsFromFile  (QWidget *parent, const std::string &filename);

  void addLayout(const QString &name, QLayout *l);
  QLayout *getLayout(const QString &name) const;

  template<typename T>
  T *getLayoutT(const QString &name) const {
    return qobject_cast<T *>(getLayout(name));
  }

  void addWidget(const QString &name, QWidget *w);
  QWidget *getWidget(const QString &name) const;

  template<typename T>
  T *getWidgetT(const QString &name) const {
    return qobject_cast<T *>(getWidget(name));
  }

  void addAction(const QString &name, QAction *action);
  QAction *getAction(const QString &name) const;

 private:
  using LayoutMap       = std::map<QString, QLayout *>;
  using WidgetMap       = std::map<QString, QWidget *>;
  using ActionMap       = std::map<QString, QAction *>;
  using WidgetFactories = std::map<QString, CQXmlWidgetFactory *>;
  using TagFactories    = std::map<QString, CQXmlTagFactory *>;

  CXML*           xml_     { nullptr };
  QWidget*        parent_  { nullptr };
  CQXmlFactory*   factory_ { nullptr };
  LayoutMap       layouts_;
  WidgetMap       widgets_;
  ActionMap       actions_;
  WidgetFactories widgetFactories_;
  TagFactories    tagFactories_;
};

#endif
