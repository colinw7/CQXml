#ifndef CQXml_H
#define CQXml_H

#include <string>
#include <map>

#include <QString>
#include <QStringList>

class CXML;
class CQXmlFactory;

class QWidget;
class QLayout;
class QAction;

class CQXmlWidgetFactory {
 public:
  CQXmlWidgetFactory() { }

  virtual ~CQXmlWidgetFactory() { }

  virtual QWidget *createWidget(const QStringList &params=QStringList()) = 0;
};

class CQXml {
 public:
  CQXml();
 ~CQXml();

  void createWidgetsFromString(QWidget *parent, const std::string &str);

  void createWidgetsFromFile(QWidget *parent, const std::string &filename);

  bool isWidgetFactory(const QString &name) const;

  void addWidgetFactory(const QString &name, CQXmlWidgetFactory *factory);

  void removeWidgetFactory(const QString &name);

  CXML *getXml() const { return xml_; }

  QWidget *parent() const { return parent_; }

  CQXmlFactory *getFactory() const { return factory_; }

  CQXmlWidgetFactory *getWidgetFactory(const QString &name) const;

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
 private:
  typedef std::map<QString, QLayout *>            LayoutMap;
  typedef std::map<QString, QWidget *>            WidgetMap;
  typedef std::map<QString, QAction *>            ActionMap;
  typedef std::map<QString, CQXmlWidgetFactory *> WidgetFactories;

  CXML            *xml_;
  QWidget         *parent_;
  CQXmlFactory    *factory_;
  LayoutMap        layouts_;
  WidgetMap        widgets_;
  ActionMap        actions_;
  WidgetFactories  widgetFactories_;
};

#endif
