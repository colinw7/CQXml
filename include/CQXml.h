#ifndef CQXml_H
#define CQXml_H

#include <string>
#include <map>

#include <QString>

class CXML;
class CQXmlFactory;

class QWidget;
class QBoxLayout;

class CQXml {
 public:
  CQXml();
 ~CQXml();

  void createWidgets(QWidget *parent, const std::string &str);

  CXML *getXml() const { return xml_; }

  CQXmlFactory *getFactory() const { return factory_; }

  void addLayout(const QString &name, QBoxLayout *l);

  QBoxLayout *getLayout(const QString &name) const;

 private:
  typedef std::map<QString, QBoxLayout *> LayoutMap;

  CXML         *xml_;
  CQXmlFactory *factory_;
  LayoutMap     layouts_;
};

#endif
