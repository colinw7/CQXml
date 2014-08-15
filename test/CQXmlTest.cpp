#include <CQXmlTest.h>
#include <CQXml.h>
#include <CQStyleControl.h>
#include <CQStyleDivider.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>

static const char *xmlStr =
"<qxml>\n"
"<h1>Heading 1</h1>\n"
"<h2>Heading 2</h2>\n"
"<h3>Heading 3</h3>\n"
"<h4>Heading 4</h4>\n"
"<p>"
"Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
"sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
"nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in\n"
"reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla\n"
"pariatur. Excepteur sint occaecat cupidatat non proident, sunt in\n"
"culpa qui officia deserunt mollit anim id est laborum."
"</p>"
"<QVBoxLayout name=\"l1\">\n"
"<QHBoxLayout name=\"l2\"><p>Edit 1</p><QLineEdit/></QHBoxLayout>\n"
"<QHBoxLayout name=\"l3\"><p>Edit 2</p><QLineEdit/></QHBoxLayout>\n"
"<QHBoxLayout name=\"l4\"><p>Edit 3</p><QLineEdit/></QHBoxLayout>\n"
"</QVBoxLayout>\n"
"</qxml>";

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQXmlTest *test = new CQXmlTest;

  if (argc > 1) {
    for (int i = 1; i < argc; ++i)
      test->loadFile(argv[i]);
  }
  else
    test->loadStr(xmlStr);

  //test->addControl();

  test->resize(500, 500);

  test->show();

  return app.exec();
}

CQXmlTest::
CQXmlTest()
{
  xml_ = new CQXml;
}

void
CQXmlTest::
loadFile(const char *filename)
{
  xml_->createWidgetsFromFile(this, filename);
}

void
CQXmlTest::
loadStr(const char *str)
{
  xml_->createWidgetsFromString(this, str);

  xml_->getLayoutT<QBoxLayout>("l2")->addWidget(new QLineEdit);
  xml_->getLayoutT<QBoxLayout>("l2")->addStretch(1);
  xml_->getLayoutT<QBoxLayout>("l3")->addWidget(new QLineEdit);
  xml_->getLayoutT<QBoxLayout>("l3")->addStretch(1);
  xml_->getLayoutT<QBoxLayout>("l4")->addWidget(new QLineEdit);
  xml_->getLayoutT<QBoxLayout>("l4")->addStretch(1);

  layout()->addWidget(new CQStyleDivider("p", CQStyleDivider::LineType));
}

void
CQXmlTest::
addControl()
{
  CQStyleControl *control = new CQStyleControl;

  layout()->addWidget(control);
}
