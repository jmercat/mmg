/** Authors Cécile Dobrzynski, Charles Dapogny, Pascal Frey and Algiane Froehly */
/** \include Example for using mmg3dlib (advanced used) */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

#include "libmmg3d5.h"

int main(int argc,char *argv[]) {
  MMG5_pMesh      mmgMesh;
  MMG5_pSol       mmgSol;
  int             k,ier;
  char            *pwd,*inname,*outname;

  fprintf(stdout,"  -- TEST MMG3DLIB \n");

  /* Name and path of the mesh files */
  pwd = getenv("PWD");
  inname = (char *) calloc(strlen(pwd) + 34, sizeof(char));
  if ( inname == NULL ) {
    perror("  ## Memory problem: calloc");
    exit(EXIT_FAILURE);
  }
  outname = (char *) calloc(strlen(pwd) + 43, sizeof(char));
  if ( outname == NULL ) {
    perror("  ## Memory problem: calloc");
    exit(EXIT_FAILURE);
  }
  sprintf(inname, "%s%s%s", pwd, "/../libexamples/example2/", "2spheres");

  /** 1) Initialisation of mesh and sol structures */
  /* args of InitMesh: mesh=&mmgMesh, sol=&mmgSol, input mesh name, input sol name,
   output mesh name */
  mmgMesh = NULL;
  mmgSol  = NULL;
  MMG5_Init_mesh(&mmgMesh,&mmgSol);

  /** 2) Build mesh in MMG5 format */
  /** Two solutions: just use the MMG5_loadMesh function that will read a .mesh(b)
     file formatted or manually set your mesh using the MMG5_Set* functions */

  /** with MMG5_loadMesh function */
  /** a) (not mandatory): give the mesh name
     (by default, the "mesh.mesh" file is oppened)*/
  if ( !MMG5_Set_inputMeshName(mmgMesh,inname) )
    exit(EXIT_FAILURE);
  /** b) function calling */
  if ( !MMG5_loadMesh(mmgMesh) )  exit(EXIT_FAILURE);

  /** 3) Build sol in MMG5 format */
  /** Two solutions: just use the MMG5_loadMet function that will read a .sol(b)
      file formatted or manually set your sol using the MMG5_Set* functions */

  /** With MMG5_loadMet function */
  /** a) (not mandatory): give the sol name
     (by default, the "mesh.sol" file is oppened)*/
  if ( !MMG5_Set_inputSolName(mmgMesh,mmgSol,inname) )
    exit(EXIT_FAILURE);
  /** b) function calling */
  if ( !MMG5_loadMet(mmgMesh,mmgSol) )
    exit(EXIT_FAILURE);

  /** 4) (not mandatory): check if the number of given entities match with mesh size */
  if ( !MMG5_Chk_meshData(mmgMesh,mmgSol) ) exit(EXIT_FAILURE);

  /** 5) (not mandatory): set your global parameters using the
      MMG5_Set_iparameters and MMG5_Set_dparameters function
      (resp. for integer parameters and double param)*/


  /**------------------- First wave of refinment---------------------*/

  /* debug mode ON (default value = OFF) */
  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_debug, 1) )
    exit(EXIT_FAILURE);

  /* memory size (default value = 801) */
  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_mem, 500) )
    exit(EXIT_FAILURE);

  /* Maximal mesh size (default FLT_MAX)*/
  if ( !MMG5_Set_dparameters(mmgMesh,mmgSol,MMG5_DPARAM_hmax,40) )
    exit(EXIT_FAILURE);

  /* Minimal mesh size (default FLT_MIN)*/
  if ( !MMG5_Set_dparameters(mmgMesh,mmgSol,MMG5_DPARAM_hmin,0.001) )
    exit(EXIT_FAILURE);

  /* Global hausdorff value (default value = 0.01) applied on the whole boundary */
  if ( !MMG5_Set_dparameters(mmgMesh,mmgSol,MMG5_DPARAM_hausd, 0.1) )
    exit(EXIT_FAILURE);

  /* Gradation control (default value 1.105) */
  if ( !MMG5_Set_dparameters(mmgMesh,mmgSol,MMG5_DPARAM_hgrad, 2) )
    exit(EXIT_FAILURE);

  /** library call */
  ier = MMG5_mmg3dlib(mmgMesh,mmgSol);
  if ( ier == MMG5_STRONGFAILURE ) {
    fprintf(stdout,"BAD ENDING OF MMG3DLIB: UNABLE TO SAVE MESH\n");
    return(ier);
  } else if ( ier == MMG5_LOWFAILURE )
    fprintf(stdout,"BAD ENDING OF MMG3DLIB\n");

  /* (Not mandatory) Automatically save the mesh */
  sprintf(outname, "%s%s%s", pwd, "/../libexamples/example2/", "2spheres_1.o.mesh");
  if ( !MMG5_Set_outputMeshName(mmgMesh,outname) )
    exit(EXIT_FAILURE);

  MMG5_saveMesh(mmgMesh);

  /* (Not mandatory) Automatically save the solution */
  if ( !MMG5_Set_outputSolName(mmgMesh,mmgSol,outname) )
    exit(EXIT_FAILURE);

  MMG5_saveMet(mmgMesh,mmgSol);


  /**------------------- Second wave of refinment---------------------*/
  /* We add different local hausdorff numbers on boundary componants (this
     local values are used instead of the global hausdorff number) */

  /* verbosity (default value = 5)*/
  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_verbose, 4) )
    exit(EXIT_FAILURE);

  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_mem, 1000) )
    exit(EXIT_FAILURE);
  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_debug, 0) )
    exit(EXIT_FAILURE);


  /** 6) (not mandatory): set your local parameters */
  /* use 2 local hausdorff numbers on ref 36 (hausd = 0.01) and 38 (hausd = 1) */
  if ( !MMG5_Set_iparameters(mmgMesh,mmgSol,MMG5_IPARAM_numberOfLocalParam,2) )
    exit(EXIT_FAILURE);

  /** for each local parameter: give the type and reference of the element on which
      you will apply a particular hausdorff number and the hausdorff number
      to apply. The global hausdorff number is applied on all boundary triangles
      without local hausdorff number */

  /* Be careful if you change the hausdorff number (or gradation value)
     between 2 run: the information of the previous hausdorff number (resp. gradation)
     is contained in the metric computed during the previous run.
     Then, you can not grow up the hausdorff value (resp. gradation) without resetting
     this metric (but you can decrease this value). */

  if ( !MMG5_Set_localParameters(mmgMesh,mmgSol,MMG5_Triangle,36,0.01) )
    exit(EXIT_FAILURE);
  if ( !MMG5_Set_localParameters(mmgMesh,mmgSol,MMG5_Triangle,38,1) )
    exit(EXIT_FAILURE);

  /** library call */
  ier = MMG5_mmg3dlib(mmgMesh,mmgSol);
  if ( ier == MMG5_STRONGFAILURE ) {
    fprintf(stdout,"BAD ENDING OF MMG3DLIB: UNABLE TO SAVE MESH\n");
    return(ier);
  } else if ( ier == MMG5_LOWFAILURE )
    fprintf(stdout,"BAD ENDING OF MMG3DLIB\n");

  /**------------------- Last wave of refinment---------------------*/
  /** library call */

  /* New metric to see the effect of the local hausdorff number on triangles
     of ref 38: constant and of size 10 */
  for ( k=1; k<=mmgSol->np; k++ ) {
    if ( !MMG5_Set_scalarSol(mmgSol,10,k) ) exit(EXIT_FAILURE);
  }

  ier = MMG5_mmg3dlib(mmgMesh,mmgSol);
  if ( ier == MMG5_STRONGFAILURE ) {
    fprintf(stdout,"BAD ENDING OF MMG3DLIB: UNABLE TO SAVE MESH\n");
    return(ier);
  } else if ( ier == MMG5_LOWFAILURE )
    fprintf(stdout,"BAD ENDING OF MMG3DLIB\n");


  /* 7) Automatically save the mesh */
  sprintf(outname, "%s%s%s", pwd, "/../libexamples/example2/", "2spheres_2.o.mesh");
  if ( !MMG5_Set_outputMeshName(mmgMesh,outname) )
    exit(EXIT_FAILURE);

  MMG5_saveMesh(mmgMesh);

  /* 8) Automatically save the solution */
  if ( !MMG5_Set_outputSolName(mmgMesh,mmgSol,outname) )
    exit(EXIT_FAILURE);

  MMG5_saveMet(mmgMesh,mmgSol);

  /* 9) free the MMG3D5 structures */
  MMG5_Free_all(mmgMesh,mmgSol);

  free(inname);
  inname = NULL;
  free(outname);
  outname = NULL;

  return(ier);
}
