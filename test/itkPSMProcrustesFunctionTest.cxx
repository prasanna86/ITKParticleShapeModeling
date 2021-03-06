/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <iostream>
#include <fstream>
#include "itkPSMProcrustesFunction.h"
#include "itkCommand.h"
#include "itkPSMProjectReader.h"

int itkPSMProcrustesFunctionTest( int argc, char* argv[] )
{
  bool passed = true;
  double value1, value2, value3;
  itk::PSMProcrustesFunction::PointType pt;
  std::string errstring = "";
  std::string output_path = "";
  std::string input_path_prefix = "";
  itk::PSMProcrustesFunction::ShapeType s;
  itk::PSMProcrustesFunction::ShapeListType sl;
  
  // Check for proper arguments
  if (argc < 2)
    {
    std::cout << "Wrong number of arguments. \nUse: "
              << "itkPSMProcrustesFunctionTest parameter_file [output_path] [input_path]\n"
              << "See itk::PSMParameterFileReader for documentation on the parameter file format.\n"
              <<" Note that input_path will be prefixed to any file names and paths in the xml parameter file.\n"
              << std::endl;
    return EXIT_FAILURE;
    }
  
  if (argc >2)
    {
    output_path = std::string(argv[2]);
    }
  
  if (argc >3)
    {
    input_path_prefix = std::string(argv[3]);
    }
  
  try
    {
    // Read the project parameters
    itk::PSMProjectReader::Pointer xmlreader = itk::PSMProjectReader::New();
    xmlreader->SetFileName(argv[1]);
    xmlreader->Update();
    
    itk::PSMProject::Pointer project = xmlreader->GetOutput();
    
    // Create the Procrustes pointer
    itk::PSMProcrustesFunction::Pointer P = itk::PSMProcrustesFunction::New();
    
    // Load the point files
    const std::vector<std::string> &pt_files = project->GetModel(std::string("initialization"));
    std::cout << "Reading the point files to be registered ..." << std::endl;
    for (unsigned int i = 0; i < pt_files.size(); i++)
      {
      // Read the points for this file and add as a list
      s.clear();
      // Open the ascii file.
      std::ifstream in;
      in.open( (input_path_prefix + pt_files[i]).c_str() );
      if ( !in )
        {
        errstring += "Could not open point file for input.";
        passed = false;
        break;
        }
      
      // Read all of the points, one point per line.
      while (in)
        {
        //itk::PSMProcrustesFunction::PointType pt;
        
        //for (unsigned int d = 0; d < 3; d++)
        //{
        //in >> pt[d];
        in>>value1>>value2>>value3;
        pt[0] = value1;
        pt[1] = value2;
        pt[2] = value3;
        s.push_back(pt);
        //}
        //s.push_back(pt);
        }
      // This algorithm pushes the last point twice
      s.pop_back();
      
      // Store the shape in the list
      sl.push_back(s);
      in.close();
      std::cout << "  " << pt_files[i] << std::endl;
      }
    /*std::cout << "SHAPES: " << std::endl;
      for(itk::PSMProcrustesFunction::ShapeListIteratorType shape_it = sl.begin(); shape_it != sl.end(); shape_it++)
      {
      for(itk::PSMProcrustesFunction::ShapeIteratorType it = (*shape_it).begin(); it != (*shape_it).end(); ++it)
      {
      itk::PSMProcrustesFunction::PointType & point = *it;
      std::cout << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
      }
      std::cout << "shape" << std::endl;
      }*/
    
    std::cout << "Done!" << std::endl;
    
    itk::PSMProcrustesFunction::SimilarityTransformListType transforms;
    itk::PSMProcrustesFunction::Pointer procrustes = itk::PSMProcrustesFunction::New();
    procrustes->RunGeneralizedProcrustes(transforms, sl);
    
    // Print out the outputs
    // Load the output model names
    const std::vector<std::string> &out_files = project->GetModel(std::string("optimized"));
    
    for (unsigned int i = 0; i < out_files.size(); i++)
      {            
      // Open the output file.
      std::string fname = output_path + out_files[i];
      std::ofstream out;
      out.open( fname.c_str() );
      s = sl[i];
      if ( !out )
        {
        errstring += "Could not open point file for output: ";
        }
      else
        {                
        for(itk::PSMProcrustesFunction::ShapeIteratorType it = s.begin(); it != s.end(); it++)
          {
          itk::PSMProcrustesFunction::PointType & point = (*it);
          out << point[0] << " " << point[1] << " " << point[2] << std::endl;
          }
        //std::cout << "printing output " << fname << std::endl;
        out.close();
        }
      }
    passed = true;
    // TODO: How to check if shapes have been registered correctly?
    }
  
  catch(itk::ExceptionObject &e)
    {
    errstring = "ITK exception with description: " + std::string(e.GetDescription())
      + std::string("\n at location:") + std::string(e.GetLocation())
      + std::string("\n in file:") + std::string(e.GetFile());
    passed = false;
    }
  catch(...)
    {
    errstring = "Unknown exception thrown";
    passed = false;
    }
  
  if (passed)
    {
    std::cout << "All tests passed" << std::endl;
    return EXIT_SUCCESS;
    }
  else
    {
    std::cout << "Test failed with the following error:" << std::endl;
    std::cout << errstring << std::endl;
    return EXIT_FAILURE;
    }
}
