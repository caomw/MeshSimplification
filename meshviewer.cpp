#define NOMINMAX

#include "meshviewer.h"
#include "glutils.hpp"
#include <fstream>
#include <string>
using namespace std;

#include <QMouseEvent>
#include <QGLFunctions>
#include <QFileDialog>

MeshViewer::MeshViewer(QWidget *parent) :
QGLWidget(qglformat_3d, parent)
{
  interactionState = Camera;
  viewerState.updateModelView();
  enableLighting = false;
  lastSelectedIndex = 0;
}

MeshViewer::~MeshViewer()
{

}

bool MeshViewer::QtUnProject(const QVector3D& pos_screen, QVector3D& pos_world)
{
  bool isInvertible;
  QMatrix4x4 proj_modelview_inv = viewerState.projectionModelView().inverted(&isInvertible);
  if (isInvertible)
  {
    QVector3D pos_camera;
    pos_camera.setX((pos_screen.x() - (float)viewerState.viewport.x) / (float)viewerState.viewport.w*2.0 - 1.0);
    pos_camera.setY((pos_screen.y() - (float)viewerState.viewport.y) / (float)viewerState.viewport.h*2.0 - 1.0);
    pos_camera.setZ(2.0*pos_camera.z() - 1.0);
    pos_world = (proj_modelview_inv*QVector4D(pos_camera, 1.0f)).toVector3DAffine();
  }

  return isInvertible;
}

void MeshViewer::computeGlobalSelectionBox()
{
  /// get GL state
  GLint m_GLviewport[4];
  GLdouble m_GLmodelview[16];
  GLdouble m_GLprojection[16];
  glGetIntegerv(GL_VIEWPORT, m_GLviewport);           // Retrieves The Viewport Values (X, Y, Width, Height)
  glGetDoublev(GL_MODELVIEW_MATRIX, m_GLmodelview);       // Retrieve The Modelview Matrix
  glGetDoublev(GL_PROJECTION_MATRIX, m_GLprojection);     // Retrieve The Projection Matrix

  //Not know why, but it solves the problem, maybe some issue with QT
  if (width() < height())
    m_GLviewport[1] = -m_GLviewport[1];

  GLdouble winX = sbox.corner_win[0];
  GLdouble winY = sbox.corner_win[1];
  QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[0]);
  //qDebug()<<sbox.gcorners[0];
  gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global, sbox.corner_global + 1, sbox.corner_global + 2);//The new position of the mouse
  //qDebug()<<sbox.corner_global[0]<<sbox.corner_global[1]<<sbox.corner_global[2];

  winX = sbox.corner_win[0];
  winY = sbox.corner_win[3];
  QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[1]);
  //qDebug()<<sbox.gcorners[1];
  gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 3, sbox.corner_global + 4, sbox.corner_global + 5);//The new position of the mouse
  //qDebug()<<sbox.corner_global[3]<<sbox.corner_global[4]<<sbox.corner_global[5];

  winX = sbox.corner_win[2];
  winY = sbox.corner_win[3];
  QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[2]);
  //qDebug()<<sbox.gcorners[2];
  gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 6, sbox.corner_global + 7, sbox.corner_global + 8);//The new position of the mouse
  //qDebug() << sbox.corner_global[6] << sbox.corner_global[7]<< sbox.corner_global[8];

  winX = sbox.corner_win[2];
  winY = sbox.corner_win[1];
  QtUnProject(QVector3D(winX, winY, 0.001), sbox.gcorners[3]);
  //qDebug()<<sbox.gcorners[3];
  gluUnProject(winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global + 9, sbox.corner_global + 10, sbox.corner_global + 11);//The new position of the mouse
  //qDebug() << sbox.corner_global[9] << sbox.corner_global[10]<< sbox.corner_global[11];
}

void MeshViewer::mousePressEvent(QMouseEvent *e)
{
  mouseState.isPressed = true;

  /// set interaction mode as camera if shift key is hold
  if (e->modifiers() & Qt::AltModifier) {
    interactionStateStack.push(interactionState);
    interactionState = Camera;
  }

  switch (interactionState) {
  case Camera:
    mouseState.prev_pos = QVector2D(e->pos());
    break;
  }
}

void MeshViewer::mouseMoveEvent(QMouseEvent *e)
{
  switch (interactionState) {
  case Camera: {
    if (e->buttons() & Qt::LeftButton) {
      QVector2D diff = QVector2D(e->pos()) - mouseState.prev_pos;

      if ((e->modifiers() & Qt::ShiftModifier)) {
        viewerState.translation += QVector3D(diff.x() / 100.0, -diff.y() / 100.0, 0.0);
      }
      else if (e->modifiers() & Qt::ControlModifier)
      {
        viewerState.translation += QVector3D(0.0, 0.0, diff.x() / 100.0 - diff.y() / 100.0);
      }
      else{
        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length() / 4.0;
        // Calculate new rotation axis as weighted sum
        viewerState.rotationAxis = (viewerState.rotationAxis * viewerState.angularChange + n * acc).normalized();
        // Change rotation angle
        viewerState.angularChange = acc;

        viewerState.rotation = QQuaternion::fromAxisAndAngle(viewerState.rotationAxis, viewerState.angularChange) * viewerState.rotation;
      }
      viewerState.updateModelView();
      mouseState.prev_pos = QVector2D(e->pos());
    }
    updateGL();
    break;
  }
  }
}

void MeshViewer::mouseReleaseEvent(QMouseEvent *e)
{
  switch (interactionState) {
  case Camera:
    mouseState.prev_pos = QVector2D(e->pos());
    break;
  }
  mouseState.isPressed = false;

  /// reset interaction mode if in camera mode triggered by holding alt
  if (e->modifiers() & Qt::AltModifier) {
    interactionState = interactionStateStack.top();
    interactionStateStack.pop();
  }

  updateGL();
}

void MeshViewer::keyPressEvent(QKeyEvent *e)
{
  switch (e->key()) {
  case Qt::Key_L:
  {
    enableLighting = !enableLighting;
    break;
  }
  case Qt::Key_M:
  {
      // load a new mesh
      QString fn = QFileDialog::getOpenFileName();
      string filename = fn.toStdString();
      if(!OpenMesh::IO::read_mesh(heMesh, filename)) {
          cerr << "Failed to open file [" << filename << "]" << endl;
      }
      else
      {
          std::cout << "# Vertices: " << heMesh.n_vertices() << std::endl;
          std::cout << "# Edges : " << heMesh.n_faces() << std::endl;
          std::cout << "# Faces : " << heMesh.n_faces() << std::endl;
      }
  }
  }
  updateGL();
}

void MeshViewer::keyReleaseEvent(QKeyEvent *e)
{

}

void MeshViewer::wheelEvent(QWheelEvent *e)
{
  switch (interactionState) {
  case Camera:{
    double numSteps = e->delta() / 200.0f;
    viewerState.translation.setZ(viewerState.translation.z() + numSteps);
    viewerState.updateModelView();
    break;
  }
  default:
    break;
  }
  updateGL();
}

void MeshViewer::enterEvent(QEvent *e)
{
  QGLWidget::enterEvent(e);
  grabKeyboard();
  setFocus();
}

void MeshViewer::leaveEvent(QEvent *e)
{
  QGLWidget::leaveEvent(e);
  releaseKeyboard();
}

void MeshViewer::initializeGL()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glShadeModel(GL_SMOOTH);

  setMouseTracking(true);
}

void MeshViewer::resizeGL(int w, int h)
{
  viewerState.updateViewport(w, h);
  viewerState.updateProjection();

  glViewport(viewerState.viewport.x, viewerState.viewport.y, viewerState.viewport.w, viewerState.viewport.h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixf(viewerState.projection.constData());
}

void MeshViewer::paintGL()
{
  //glEnable(GL_DEPTH_TEST);
  glClearColor(1., 1., 1., 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // the model view matrix is updated somewhere else
  glMultMatrixf(viewerState.modelview.constData());

  glEnable(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);

    if (enableLighting)
      enableLights();

    // draw the mesh
    for(auto fit = heMesh.faces_sbegin(); fit != heMesh.faces_end(); ++fit) {
        // draw this face
        glBegin(GL_POLYGON);
        for(auto fvit = heMesh.fv_iter(*fit);fvit.is_valid();++fvit) {
            Mesh::Point p = heMesh.point(*fvit);
            cout << p[0] << ',' << p[1] << ',' << p[2] << endl;
        }
        glEnd();
    }

    if (enableLighting)
      disableLights();
}

static QImage toQImage(const unsigned char* data, int w, int h) {
  QImage qimg(w, h, QImage::Format_ARGB32);
  for (int i = 0, idx = 0; i < h; i++) {
    for (int j = 0; j < w; j++, idx += 4)
    {
      unsigned char r = data[idx + 2];
      unsigned char g = data[idx + 1];
      unsigned char b = data[idx];
      unsigned char a = 255;
      QRgb qp = qRgba(r, g, b, a);
      qimg.setPixel(j, i, qp);
    }
  }
  return qimg;
}

void MeshViewer::enableLights()
{
  GLfloat light_position[] = { 10.0, 4.0, 10.0, 1.0 };
  GLfloat mat_specular[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat mat_diffuse[] = { 0.375, 0.375, 0.375, 1.0 };
  GLfloat mat_shininess[] = { 25.0 };
  GLfloat light_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
  GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

  light_position[0] = -10.0;
  glLightfv(GL_LIGHT1, GL_POSITION, light_position);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
}

void MeshViewer::disableLights()
{
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHTING);
}
