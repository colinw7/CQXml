#include <QDialog>

class CQXml;

class CQXmlTest : public QDialog {
  Q_OBJECT

 public:
  CQXmlTest();

  void loadFile(const char *filename);
  void loadStr(const char *str);

  void addControl();

 private:
  CQXml *xml_;
};
