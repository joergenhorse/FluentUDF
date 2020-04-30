#include "udf.h"
#include "string.h"
#include <stdio.h>

#define CopperStartAxial 0.004441
#define CopperStartRadial 0.334371 
#define CopperDimensionAxial 0.014275
#define CopperDimensionRadial 0.002057
#define Paper 0.000762 /* double sided paper */
#define Spacer 0.00406 /* spacer */
#define Oilguide 0.00889/* oilguide */
#define Radialn 5  /* number of strands */
#define Axialn 4  /* number of discs */

static float Sources [Axialn][Radialn];
static int ReadInFile = 0;

void ReadPowerLossFile()
{
#if !RP_HOST
/***********************************
 * Index system
 ***********************************/
   int cnt1, cnt2;
   char mychar [255];
   char * pch;
   FILE *fp;

   cnt1 = 0;
   cnt2 = 0;

   if( (fp = fopen("disc_power_loss.csv","r")) == NULL )
	   Message("File not opened\n");
   else
	   Message("File opened\n");

   while (!feof(fp))
   {
		fscanf (fp, "%s", mychar);
		pch = strtok (mychar,",");
		cnt2 = 0;

		while (pch != NULL)
		{
			Sources[cnt1][cnt2] = atof(pch);
			pch = strtok (NULL, ",");
			cnt2 += 1;
		}
		cnt1 += 1;
   }
	fclose(fp);
	Message("File closed\n");

#endif
}

DEFINE_ON_DEMAND(dod)
{
#if !RP_HOST
   int cnt1, cnt2;
   char mychar [255];
   char * pch;
   FILE *fp;

   cnt1 = 0;
   cnt2 = 0;

   if( (fp = fopen("disc_power_loss.csv","r")) == NULL )
	   Message("File not opened\n");
   else
	   Message("File opened\n");

   while (!feof(fp))
   {
		fscanf (fp, "%s", mychar);
		pch = strtok (mychar,",");
		cnt2 = 0;

		while (pch != NULL)
		{
			Sources[cnt1][cnt2] = atof(pch);
			pch = strtok (NULL, ",");
			cnt2 += 1;
		}
		cnt1 += 1;
   }
	fclose(fp);
	Message("File closed\n");

#endif
}

DEFINE_SOURCE(energy, cell, thread, dS, eqn)
/*DEFINE_PROFILE(fixed_temp, thread, nv) */
/*/DEFINE_ON_DEMAND(dod) */
{
#if !RP_HOST
   double rAxial,rRadial;
   double pos[ND_ND];
   double vol;
   double source;
   /*/char str [100];*/
   int radialIndex;
   int axialIndex;
   int dt;
   float radialStart, radialEnd;
   float axialStart, axialEnd;
   int found;
   float volcalc;
   FILE* fp;


   if (ReadInFile!=1)
   {
	   ReadPowerLossFile();
	   Message("ReadInFile = %d\n", ReadInFile);
	   ReadInFile = 1;
   }
   

   C_CENTROID(pos,cell,thread);
   rAxial=pos[0];
   rRadial=pos[1];

   radialIndex = 0;
   axialIndex = 0;
   
   /*************************************************
    * Find radialIndex, the index for the radial (y) winding
	*************************************************/
   /* initialize radialEnd reference. 
    * The first time innerStrand is calculated it needs the radialEnd to give the correct dimension, 
	* so this is a non-physical quantity. It is the outside diameter of the ghost strand before the first strand... */
   radialEnd = CopperStartRadial-Paper; 
	
	/* Initialize other variables */
   dt = 0;
   found = 0;
   /*Message("before loop %i %i\n",found,dt);*/
   while ((dt < Radialn) && (found < 1))
   {
	   dt = dt + 1; /* counter, counts up til number of strands Radialn */
	    
	   
	   
	   radialStart = radialEnd + Paper; /* inside diameter of copper strand */
	   radialEnd = radialStart + CopperDimensionRadial; /* outside diameter of copper strand */

/*/    Message("%i %i %f %f %f\n",found,dt,tmp,tmp2,rAxial);*/
    
	   if (radialEnd > radialStart)
	   {
		   if ((rRadial < radialEnd) & (rRadial > radialStart))  /* check cell is in strand */
		   {
			   radialIndex = dt;
			   found = 1;
			   
		   }
	   }

   }
  

   /*************************************************
    * Find axialIndex, the index for the axial (x) winding
	*************************************************/
	/* Initialize axialEnd reference
    * The first time axialStart is calculated it needs the toStrand to give the correct dimension, 
	* so this is a non-physical. It is the top level of the ghost strand below the first strand... */
   axialEnd = CopperStartAxial-(Spacer + Paper);

	/* Initialize other variables */
   dt = 0;
   found = 0;
   while ((dt < Axialn) && (found < 1))
   {
	   dt = dt + 1; /* counter, counts up til number of discs Axialn */

	   /* The first time this must give the height of the bottom of the first strand
		*/

	   axialStart = axialEnd + (Spacer + Paper); /* height at bottom of first copper disc */
	      
	   axialEnd = axialStart + CopperDimensionAxial; /* height at top of copper disc */
	   
/*/    Message("%i %i %f %f %f\n",found,dt,tmp,tmp2,rRadial);*/

	   if (axialEnd > axialStart)
	   {
		   if ((rAxial < axialEnd) & (rAxial > axialStart))
		   {
			   axialIndex = dt;
			   found = 1;
		   }
	   }

   }
 

      /*************************************************
    * Calculate volume of 1 disc (360 degs)
	*************************************************/
   volcalc = CopperDimensionAxial*3.14159265*fabs(pow(radialEnd,2.0)-pow(radialStart,2.0)); /* fabs - absolute value */ 


   /*************************************************
    * Divide w/winding by volume/winding
	*************************************************/
 dS[eqn] = 0.0; /* Not sure what this is? */
   source = Sources[axialIndex-1][radialIndex-1]/volcalc;

   
  
	if( (fp = fopen("UDF_output.txt","a")) == NULL )
	Message("Write file not opened\n");
	else
	{
		/*Message("Write file opened\n");*/
		fprintf(fp, "%e\t%e\t%e\t%i\t%i\n", pos[0], pos[1], source, (axialIndex-1),(radialIndex-1));
		fclose(fp);
	}
	/*Message("%e\t%e\t%e\t%i\t%i\n", pos[0], pos[1], source, (axialIndex-1),(radialIndex-1));*/
	
 Message("%f\n",source);
   return source;
   

#endif   
}
