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
"<vlayout name=\"l1\">\n"
"<hlayout name=\"l2\"><p>Edit 1</p></hlayout>\n"
"<hlayout name=\"l3\"><p>Edit 2</p></hlayout>\n"
"<hlayout name=\"l4\"><p>Edit 3</p></hlayout>\n"
"</vlayout>\n"
"</qxml>";

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQXmlTest *test = new CQXmlTest;

  test->resize(500, 500);

  test->show();

  return app.exec();
}

CQXmlTest::
CQXmlTest()
{
  xml_ = new CQXml;

  xml_->createWidgets(this, xmlStr);

  xml_->getLayout("l2")->addWidget(new QLineEdit);
  xml_->getLayout("l3")->addWidget(new QLineEdit);
  xml_->getLayout("l4")->addWidget(new QLineEdit);

  layout()->addWidget(new CQStyleDivider("p", CQStyleDivider::LineType));

  CQStyleControl *control = new CQStyleControl;

  layout()->addWidget(control);
}
