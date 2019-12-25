#include <Quarter/Quarter.h>
#include <Quarter/QuarterWidget.h>

#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoSelection.h>
#include <Inventor/actions/SoBoxHighlightRenderAction.h>

#include <QtGui/QApplication>

using namespace SIM::Coin3D::Quarter;

int main(int argc, char *argv[]) {
  // init
  QApplication app(argc, argv);
  Quarter::init();

  bool boxhlight = (argc > 1 && strcmp(argv[1],"--box")==0);
  
  // setup scene
  auto root = new SoSeparator;
  root->ref();
  
  SoSelection * sel = new SoSelection;
  root->addChild(sel);

  sel->addSelectionCallback(
    [] (void *data,SoPath *) -> void {
      static_cast<SoSelection*>(data)->touch(); // to redraw
    }
    ,root);
  sel->addDeselectionCallback(
    [] (void *data,SoPath *) -> void {
      static_cast<SoSelection*>(data)->touch(); // to redraw
    }
    ,root);

  sel->addChild(new SoCone);
  
  // set up viewer
  auto myViewer = new QuarterWidget;
  myViewer->setSceneGraph(root);
  
  // make the viewer react to input events similar to 'old' ExaminerViewer
  myViewer->setNavigationModeFile(QUrl("coin:///scxml/navigation/examiner.xml"));

  SoBoxHighlightRenderAction *render = nullptr;
  if (boxhlight) { // apply BoxHighlight
    myViewer->getSoRenderManager()->setGLRenderAction(render = new SoBoxHighlightRenderAction);
  }

  // pop up Widget
  myViewer->show();
  
  // loop until exit.
  app.exec();
  
  // clean up resources.
  delete myViewer;
  root->unref();
  delete render;
  
  Quarter::clean();
  return 0;
}

