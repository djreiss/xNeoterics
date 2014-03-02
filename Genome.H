/* Genome.H - genome class defs for xNeoterics alife simulation

   Copyright (C) 2003 astrodud
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef __GENOME_H__
#define __GENOME_H__

#define NUM_OTHER_BYTES 16 // The min. # of bytes before the sensors: SHOULD BE A MULTIPLE OF 4

#undef VARY_NNETS
#ifdef VARY_NNETS
#define NUMSENSORS_DEFAULT 255 // Default tells it to set these values randomly
#define NUMNEURONS_DEFAULT 255
#define NUMOUTPUTS_DEFAULT 255

#define MAX_SENSORS 15
#define MIN_SENSORS 1
#define MAX_OUTPUTS 6
#define MIN_OUTPUTS 1
#define MAX_NEURONS 15
#define MIN_NEURONS 1
#else
#define NUMSENSORS_DEFAULT 8
#define NUMNEURONS_DEFAULT 12
#define NUMOUTPUTS_DEFAULT 6
#endif

#undef PROB_IN_GENOME // If defined, prob_mutation and prob_crossover are in genome and can change
#define MIN_PROB 1 // Minimum prob of mutation/crossver (1->255)...zero prob is bad.

#define int short

class Neoterics;

class Genome { //  The genome consists of a byte array and some size parameters
public:
   unsigned char *data; // = NULL;
   long num_bits;
   long num_bytes;

   unsigned char num_sensors_outputs; // Holds # of sensors in low 4-bits, outputs in high 4-bits
   unsigned char num_neurons; // Currently not in the actual genome, but hopefully soon!

   unsigned char *metabolism; // = NULL;
   unsigned char *maximum_mass; // = NULL;
   unsigned char *plant_eff; // = NULL;
   unsigned char *flesh_eff; // = NULL;
   unsigned char *waste_eff; // = NULL;
   unsigned char *attack_defend; // = NULL;
   unsigned char *color1; // = NULL;
   unsigned char *color2; // = NULL;
   unsigned char *prob_mut; // = NULL;
   unsigned char *prob_cross; // = NULL;
   unsigned char *min_age_reproduce; // = NULL;
   unsigned char *min_energy_reproduce; // = NULL;
   unsigned char *noise_mask; // = NULL;
   unsigned char *sensors; // = NULL;
   signed char *input_wgts;
   unsigned char *intermed_thresh; // = NULL;
   signed char *output_wgts;

   Neoterics *neo; // = NULL;
   
   static float fDistributionConstant;
	
   //  Construction/Destruction routines
public:
   Genome( Neoterics *owner,  
	   unsigned char numsensors = NUMSENSORS_DEFAULT, 
	   unsigned char numneurons = NUMNEURONS_DEFAULT, 
	   unsigned char numoutputs = NUMOUTPUTS_DEFAULT );
   Genome( Neoterics *owner, Genome *father, Genome *mother = NULL );

   void Initialize( unsigned char numsensors = NUMSENSORS_DEFAULT, 
		    unsigned char numneurons = NUMNEURONS_DEFAULT, 
		    unsigned char numoutputs = NUMOUTPUTS_DEFAULT );
   void Randomize();
   
   ~Genome();
   void ClearMemory();

   unsigned char *GetGenome();
   long GetBitLength();
   long GetByteLength();
   unsigned char NumSensors();
   unsigned char NumNeurons();
   unsigned char NumOutputs();
   float ProbMutation();
   float ProbCrossover();
	
   int ComputeOutput( int *inputs );
   void TrainNetwork();
	
   void Crossover(Genome *father, Genome *mother, float probability);
   void Mutate(float probability);
   void CopyFrom( Genome *source, long nbytes = -1, long offset = 0 );
   void InsertDelete( float probability );
#ifdef VARY_NNETS
   void AddSensor( int index );
   void AddNeuron( int index );
   void AddOutput( int index );
   void DeleteSensor( int index );
   void DeleteNeuron( int index );
   void DeleteOutput( int index );
#endif
   static float GetDistributionConstant();
   
   void GetSensorString( unsigned char sensor, char *string );
   void Output();

   void ReadFromFile( FILE *file );
   void WriteToFile( FILE *file );
};

inline unsigned char *Genome::GetGenome() {
   return data;
}

inline unsigned char Genome::NumSensors() { // numsensors goes from 1->15
   return ( num_sensors_outputs & 15 );
}

inline unsigned char Genome::NumNeurons() { // numneurons from 1->31
   return ( num_neurons & 31 );
}

inline unsigned char Genome::NumOutputs() { // numoutputs 1->7
   return ( ( num_sensors_outputs >> 4 ) & 7 );
}

inline long Genome::GetBitLength() {
   return num_bits;
}

inline long Genome::GetByteLength() {
   return num_bytes;
}

inline float Genome::ProbMutation() {
   return (float) *(prob_mut) / 10000.0;
}

inline float Genome::ProbCrossover() {
   return (float) *(prob_cross) / 10000.0;
}

#endif
