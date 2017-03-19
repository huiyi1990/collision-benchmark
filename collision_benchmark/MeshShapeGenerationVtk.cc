#include <collision_benchmark/MeshShapeGenerationVtk.hh>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkConeSource.h>
#include <vtkDiskSource.h>

#include <vtkParametricFunctionSource.h>
#include <vtkParametricEllipsoid.h>
#include <vtkParametricTorus.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>
#include <vtkAlgorithmOutput.h>

void test(vtkPolyData* polydata)
{
  std::cout << "There are " << polydata->GetNumberOfPoints()
            << " points." << std::endl;
  std::cout << "There are " << polydata->GetNumberOfPolys()
            << " triangles." << std::endl;

  for(vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
  {
    // This is identical to:
    // polydata->GetPoints()->GetPoint(i,p);
    double p[3];
    polydata->GetPoint(i,p);
    std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2] << ")" << std::endl;
  }

  // Write all of the coordinates of the points in the vtkPolyData to the console.
  polydata->GetPolys()->InitTraversal();
  vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
  while(polydata->GetPolys()->GetNextCell(idList))
  // for(vtkIdType i = 0; i < polydata->GetNumberOfPolys(); i++)
  {
    // std::cout << "Size: "<< polydata->GetPolys()->GetSize() << std::endl;
    std::cout << "Poly has " << idList->GetNumberOfIds() << " points." << std::endl;

    for(vtkIdType pointId = 0; pointId < idList->GetNumberOfIds(); pointId++)
    {
      std::cout << idList->GetId(pointId) << " ";
    }
    std::cout << std::endl;
  }
}

vtkSmartPointer<vtkPolyData> triangulate(vtkAlgorithmOutput* data)
{
  vtkSmartPointer<vtkTriangleFilter> triangleFilter =
        vtkSmartPointer<vtkTriangleFilter>::New();
  triangleFilter->SetInputConnection(data);
  triangleFilter->Update();
  return triangleFilter->GetOutput();
}

///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeSphereVtk(const unsigned int theta,
                                   const unsigned int phi,
                                   const bool latLongTessel)
{
  vtkSmartPointer<vtkSphereSource> sphereSource =
    vtkSmartPointer<vtkSphereSource>::New();

  sphereSource->SetThetaResolution(theta);
  sphereSource->SetPhiResolution(phi);
  sphereSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);
  sphereSource->SetLatLongTessellation(latLongTessel);

  // does the generation after re-setting the parameters
  sphereSource->Update();

  vtkSmartPointer<vtkPolyData> polySphere =
    triangulate(sphereSource->GetOutputPort());
  return polySphere;
}


///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeCylinderVtk(const double radius, const double height,
                                     const unsigned int resolution,
                                     const bool capping)
{

  vtkSmartPointer<vtkCylinderSource> cylinderSource =
      vtkSmartPointer<vtkCylinderSource>::New();
  cylinderSource->SetCenter(0.0, 0.0, 0.0);
  cylinderSource->SetRadius(radius);
  cylinderSource->SetHeight(height);
  cylinderSource->SetResolution(resolution);
  cylinderSource->SetCapping(capping);
  cylinderSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  cylinderSource->Update();

  vtkSmartPointer<vtkPolyData> polyCylinder =
    triangulate(cylinderSource->GetOutputPort());
  return polyCylinder;
}

///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeBoxVtk(const double x, const double y, const double z)
{
  vtkSmartPointer<vtkCubeSource> boxSource =
      vtkSmartPointer<vtkCubeSource>::New();
  boxSource->SetCenter(0.0, 0.0, 0.0);
  boxSource->SetXLength(x);
  boxSource->SetYLength(y);
  boxSource->SetZLength(z);
  // boxSource->SetBounds(xMin, xMax, yMin, yMax, zMin, zMax);
  boxSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  boxSource->Update();

  vtkSmartPointer<vtkPolyData> polyBox =
    triangulate(boxSource->GetOutputPort());
  return polyBox;
}

///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeBoxVtk(const double xMin, const double xMax,
                                const double yMin, const double yMax,
                                const double zMin, const double zMax)
{
  vtkSmartPointer<vtkCubeSource> boxSource =
      vtkSmartPointer<vtkCubeSource>::New();
  boxSource->SetCenter(0.0, 0.0, 0.0);
  boxSource->SetBounds(xMin, xMax, yMin, yMax, zMin, zMax);
  boxSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  boxSource->Update();

  vtkSmartPointer<vtkPolyData> polyBox =
    triangulate(boxSource->GetOutputPort());
  return polyBox;
}


///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeConeVtk(const double radius, const double height,
            const unsigned int resolution, const double angle_deg,
            const bool capping, const double dir_x,
            const double dir_y, const double dir_z)
{
  vtkSmartPointer<vtkConeSource> coneSource =
      vtkSmartPointer<vtkConeSource>::New();
  coneSource->SetCenter(0.0, 0.0, 0.0);
  coneSource->SetRadius(radius);
  coneSource->SetHeight(height);
  coneSource->SetResolution(resolution);
  coneSource->SetDirection(dir_x, dir_y, dir_z);
  coneSource->SetAngle(angle_deg);
  coneSource->SetCapping(capping);
  coneSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  coneSource->Update();

  vtkSmartPointer<vtkPolyData> polyCone =
    triangulate(coneSource->GetOutputPort());
  return polyCone;
}

///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeDiskVtk(const double innerRadius,
                                 const double outerRadius,
                                 const unsigned int radialResolution,
                                 const unsigned int circumResolution)
{
  vtkSmartPointer<vtkDiskSource> diskSource =
      vtkSmartPointer<vtkDiskSource>::New();
  diskSource->SetInnerRadius(innerRadius);
  diskSource->SetOuterRadius(outerRadius);
  diskSource->SetRadialResolution(radialResolution);
  diskSource->SetCircumferentialResolution(circumResolution);

  diskSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  diskSource->Update();

  vtkSmartPointer<vtkPolyData> polyDisk =
    triangulate(diskSource->GetOutputPort());
  return polyDisk;
}


///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeEllipsoidVtk(const double xRad,
                                      const double yRad,
                                      const double zRad,
                                      const unsigned int uRes,
                                      const unsigned int vRes)
{
  vtkSmartPointer<vtkParametricEllipsoid> ellipsoid =
      vtkSmartPointer<vtkParametricEllipsoid>::New();
  ellipsoid->SetXRadius(xRad);
  ellipsoid->SetYRadius(yRad);
  ellipsoid->SetZRadius(zRad);

  vtkSmartPointer<vtkParametricFunctionSource> ellipsoidSource =
      vtkSmartPointer<vtkParametricFunctionSource>::New();
  ellipsoidSource->SetParametricFunction(ellipsoid);
  ellipsoidSource->SetUResolution(uRes);
  ellipsoidSource->SetVResolution(vRes);
  ellipsoidSource->SetGenerateNormals(0);
  ellipsoidSource->SetGenerateTextureCoordinates(0);

  ellipsoidSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  ellipsoidSource->Update();

  vtkSmartPointer<vtkPolyData> polyEllipsoid =
    triangulate(ellipsoidSource->GetOutputPort());
  return polyEllipsoid;
}

///////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkPolyData>
collision_benchmark::makeTorusVtk(const double ringRadius,
                                  const double crossRadius,
                                  const unsigned int uRes,
                                  const unsigned int vRes)
{
  vtkSmartPointer<vtkParametricTorus> torus =
      vtkSmartPointer<vtkParametricTorus>::New();
  torus->SetRingRadius(ringRadius);
  torus->SetCrossSectionRadius(crossRadius);

  vtkSmartPointer<vtkParametricFunctionSource> torusSource =
      vtkSmartPointer<vtkParametricFunctionSource>::New();
  torusSource->SetParametricFunction(torus);
  torusSource->SetUResolution(uRes);
  torusSource->SetVResolution(vRes);
  torusSource->SetGenerateNormals(0);
  torusSource->SetGenerateTextureCoordinates(0);

  torusSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  // does the generation after re-setting the parameters
  torusSource->Update();

  vtkSmartPointer<vtkPolyData> polyTorus =
    triangulate(torusSource->GetOutputPort());
  return polyTorus;
}


int main()
{
  ////////////////// Primitives         /////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  // http://www.vtk.org/Wiki/VTK/Examples/Cxx/GeometricObjects/GeometricObjectsDemo

  std::cout << " +++++++++++++ SPHERE +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polySphere = collision_benchmark::makeSphereVtk(4,4,true);
  test(polySphere);

  std::cout << " +++++++++++++ CYLINDER+++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyCylinder = collision_benchmark::makeCylinderVtk(1.0, 2.0, 5, true);
  test(polyCylinder);

  std::cout << " +++++++++++++ BOX +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyBox = collision_benchmark::makeBoxVtk(1.0,2.0,3.0);
  test(polyBox);

  std::cout << " +++++++++++++ CONE +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyCone = collision_benchmark::makeConeVtk(2.0, 3.0, 5, 45, true);
  test(polyCone);

  std::cout << " +++++++++++++ DISK +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyDisk = collision_benchmark::makeDiskVtk(1.0, 5.0, 1, 5);
  test(polyDisk);


  ////////////////// Parametric objects /////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  // see also http://www.vtk.org/Wiki/VTK/Examples/Cxx/GeometricObjects/ParametricObjects
  ///////////////////////////////////////////////////////////////////////

  std::cout << " +++++++++++++ ELLIPSOID +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyEllipsoid = collision_benchmark::makeEllipsoidVtk(1.0, 2.0, 3.0,
                                                             10, 10);
  test(polyEllipsoid);

  std::cout << " +++++++++++++ TORUS +++++++++++++++ " << std::endl;
  vtkSmartPointer<vtkPolyData> polyTorus = collision_benchmark::makeTorusVtk(1, 0.1, 10, 10);
  test(polyTorus);

  return EXIT_SUCCESS;
}

