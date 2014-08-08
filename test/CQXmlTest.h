#include <QDialog>

class CQXml;

class CQXmlTest : public QDialog {
  Q_OBJECT

 public:
  CQXmlTest();

 private:
  CQXml *xml_;
};
