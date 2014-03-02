/* Genome.C - genome class for xNeoterics alife simulation

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Genome.H"
#include "Neoterics.H"
#include "Creature.H"

#define int short
#define Boolean int
#define true 1
#define false 0

Genome::Genome( Neoterics *owner,
		unsigned char numsensors, unsigned char numneurons, unsigned char numoutputs ) {
   neo = owner;

#ifdef VARY_NNETS
   int random = 0;
   if ( numsensors == NUMSENSORS_DEFAULT ) {
      while ( random < MIN_SENSORS || random > MAX_SENSORS ) random = rand() & 0x00FF;
      numsensors = random;
   }
   if ( numoutputs == NUMOUTPUTS_DEFAULT ) {
      random = 0;
      while ( random < MIN_OUTPUTS || random > MAX_OUTPUTS ) random = rand() & 0x00FF;
      numoutputs = random;
   }
   if ( numneurons == NUMNEURONS_DEFAULT ) {
      random = 0;
      while ( random < MIN_NEURONS || random > MAX_NEURONS ) random = rand() & 0x00FF;
      numneurons = random;
   }
#endif
   
   Initialize( numsensors, numneurons, numoutputs );

   Randomize();
   *prob_mut = (unsigned char) ( (float) neo->prob_mutation * 10000.0); // Encode the default into the genome
   *prob_cross = (unsigned char) ( (float) neo->prob_crossover * 10000.0);
}

Genome::Genome( Neoterics *owner, Genome *dad, Genome *mom ) {
   neo = owner;
   Initialize( dad->NumSensors(), dad->NumNeurons(), dad->NumOutputs() );
   if ( mom != NULL ) {
      Crossover( dad, mom, dad->ProbCrossover() );
      Mutate( mom->ProbMutation() );
   } else {
      //CopyFrom( dad ); // Copy the genome
      memcpy( data, dad->GetGenome(), (size_t) dad->GetByteLength() );
      Mutate( dad->ProbMutation() );
   }
#ifdef PROB_IN_GENOME // This means it is allowed to mutate to a different value
   if ( *prob_mut < MIN_PROB ) *prob_mut = MIN_PROB; // A prob of zero is inevitable if we dont do this
   if ( *prob_cross < MIN_PROB ) *prob_cross = MIN_PROB; // Besides, its not physically realistic
#else // Dont let it mutate away from the default
   *prob_mut = (unsigned char) ( (float) neo->prob_mutation * 10000.0); // Encode the default into the genome
   *prob_cross = (unsigned char) ( (float) neo->prob_crossover * 10000.0);
#endif

#ifdef VARY_NNETS
   if ( mom != NULL || dad != NULL ) InsertDelete( neo->prob_insert_delete );
#endif
}

Genome::~Genome() {
   ClearMemory();
}

void Genome::ClearMemory() {
   if ( data == NULL ) return;
   free( data );
   data = NULL;
   sensors = NULL;
   input_wgts = NULL;
   intermed_thresh = NULL;
   output_wgts = NULL;
}

float Genome::fDistributionConstant = GetDistributionConstant();

void Genome::Initialize( unsigned char numsensors, unsigned char numneurons, unsigned char numoutputs ) {
   data = NULL;
   ClearMemory();
   num_sensors_outputs = ( numsensors & 15 ) | ( ( numoutputs & 7 ) << 4 );
   num_neurons = numneurons & 31;
   num_bits = ( NUM_OTHER_BYTES + (long) numsensors + (long) numneurons + 
		(long) numsensors * (long) numneurons +
		(long) numneurons * (long) numoutputs ) * CHAR_BIT;
   num_bytes = (num_bits + 7) / 8;
   data = (unsigned char *) malloc( (size_t) num_bytes );
   metabolism = data;
   maximum_mass = data + 1;
   plant_eff = data + 2;
   flesh_eff = data + 3;
   waste_eff = data + 4;
   attack_defend = data + 5;
   color1 = data + 6;
   color2 = data + 7;
   prob_mut = data + 8;
   prob_cross = data + 9;
   min_age_reproduce = data + 10;
   min_energy_reproduce = data + 11;
   noise_mask = data + 12;
   sensors = data + NUM_OTHER_BYTES;
   input_wgts = (signed char *) ( data + NUM_OTHER_BYTES + numsensors );
   intermed_thresh = data + NUM_OTHER_BYTES + numsensors + (long) numsensors * (long) numneurons;
   output_wgts = (signed char *) data + NUM_OTHER_BYTES + numsensors +
      (long) numsensors * (long) numneurons + numneurons;
}

void Genome::Randomize() {
   unsigned char *ptr = data;
   for (long i=0; i<num_bytes; i++) {
      int random = rand() & 0x00FF;
      *ptr = (unsigned char) random;
      ptr++;
   }
}

unsigned long temp_intermediates[32];
int Genome::ComputeOutput( int *inputs ) {
   unsigned char NUMNEURONS = NumNeurons(), NUMSENSORS = NumSensors(), NUMOUTPUTS = NumOutputs();
   unsigned long *intermediates = temp_intermediates; //(long *) malloc( (size_t) ( sizeof( long ) * NUMNEURONS ) );
   signed char *weight = input_wgts; //  Calculate the values of the intermediate neurons
   unsigned char *ptr = intermed_thresh;
   unsigned long *intermediate_ptr = intermediates;
   for (int neuron=0; neuron<NUMNEURONS; neuron++) {
      long total = 0;
      int *input_ptr = inputs;
      for (int from=0; from<NUMSENSORS; from++) total += *input_ptr++ * (*weight++);
      *intermediate_ptr = 0;
      if (total >= ((unsigned int)*ptr << 4)) *intermediate_ptr = 0xffffffff;
      ptr++;
      intermediate_ptr++;
   }
   
   long max = -999999999; //  Find the highest activated output neuron
   weight = output_wgts;
   int action;
   for (int neuron=0; neuron<NUMOUTPUTS; neuron++) {
      long total = 0;
      intermediate_ptr = intermediates;
      for (int from=0; from<NUMNEURONS; from++) total += *intermediate_ptr++ & *weight++;
      if (total > max) {
	 max = total;
	 action = neuron;
      }
   }
   return action;
}

int temp_input2[32];
Boolean temp_intermediates2[32];
void Genome::TrainNetwork() { //  Routine to modify network towards goal (cheat!)
   unsigned char NUMNEURONS = NumNeurons(), NUMSENSORS = NumSensors(), NUMOUTPUTS = NumOutputs();
   int *input = temp_input2; //malloc( (size_t) ( NUMSENSORS * sizeof( int ) ) );
   Boolean *intermediates = temp_intermediates2; //malloc( (size_t) ( NUMNEURONS * sizeof( Boolean ) ) );
   int i, neuron, from;
   long total;

   *metabolism &= 0x1F; //  Fudge the initial metabolism to between 10 and 31
   if (*metabolism < 10) *metabolism = 10;
   *waste_eff &= 0x1F; // Do the same for their waste metabolism too (but from 0 to 31)
   *min_age_reproduce &= 63; // This way the min age to reproduce maxes at about 500 to start
   *min_energy_reproduce &= 63; // This way the min energy to reproduce maxes at about 500 to start

   if ( NumOutputs() > eat ) {
      sensors[0] = 12; // Create current food sensor and update the eat neuron weights
      for (i=1; i<NUMSENSORS; i++) input[i] = 0; //  Set up the inputs for positive feedback
      input[0] = 255;
      for (neuron=0; neuron<NUMNEURONS; neuron++) {
	 total = 0;
	 for (from=0; from<NUMSENSORS; from++) total += input[from] * input_wgts[from + neuron*NUMSENSORS];
	 intermediates[neuron] = false;
	 if (total >= ((unsigned int)intermed_thresh[neuron] << 2)) intermediates[neuron] = true;
      }	
      for (int from=0; from<NUMNEURONS; from++) { //  Update the output neuron weights
	 if ( intermediates[from] ) output_wgts[eat * NUMNEURONS + from] = 127; 
	 else output_wgts[eat * NUMNEURONS + from] = 0; 
      }
   }

   if ( NumOutputs() > moveForward ) {
      sensors[1] = 7; // Create forward food sensor and update the move forward neuron weights
      for (i=0; i<NUMSENSORS; i++) input[i] = 0; //  Set up the inputs for positive feedback
      input[1] = 255;
      for (neuron=0; neuron<NUMNEURONS; neuron++) {
	 total = 0;
	 for (from=0; from<NUMSENSORS; from++) total += input[from] * input_wgts[from+neuron*NUMSENSORS];
	 intermediates[neuron] = false;
	 if (total >= ((unsigned int)intermed_thresh[neuron] << 2)) intermediates[neuron] = true;
      }	
      for (from=0; from<NUMNEURONS; from++) { //  Update the output neuron weights
	 if(intermediates[from]) output_wgts[moveForward * NUMNEURONS + from] = 127; 
      }
   }

   if ( !neo->bAllowAsexual && neo->bAllowSexual && NumOutputs() > breed ) {
      sensors[2] = 12 | ( 7 << 5 ); //  If sexual only reproduction, fudge breeding when creature present
      for (i=0; i<NUMSENSORS; i++) input[i] = 0; //  Set up the inputs for positive feedback
      input[2] = 255;
      for (neuron=0; neuron<NUMNEURONS; neuron++) {
	 total = 0;
	 for (from=0; from<NUMSENSORS; from++) total += input[from] * input_wgts[from + neuron*NUMSENSORS];
	 intermediates[neuron] = false;
	 if (total >= ((unsigned int)intermed_thresh[neuron] << 2)) intermediates[neuron] = true;
      }
      for (from=0; from<NUMNEURONS; from++) { //  Update the output neuron weights
	 if (intermediates[from]) output_wgts[breed * NUMNEURONS + from] = 127; 
	 else output_wgts[breed * NUMNEURONS + from] = 0; 
      }
   }
}

void Genome::Crossover(Genome *father, Genome *mother, float probability) {
   long cross_bit, cross_byte, num_crossovers;
   long *cross_loc_bits, *cross_loc_bytes;
   int bit, i, j, index;
   unsigned char value, bit_mask;
   unsigned char *dest, *source;
   float distribution, range;

   //CopyFrom( father );
   memcpy( data, father->GetGenome(), (size_t) num_bytes ); 
   if (probability == 0.0) return;
	
   if (probability == 1.0) { // We have one crossover site if probability =1.0
      num_crossovers = 1;
   } else { //  Roll the dice to see if we have crossover, and how may
      distribution = 0.0;
      for (i=0; i<16; i++) distribution += (float)(rand() & 0x7fff);
      distribution *= fDistributionConstant;
      num_crossovers = (long)(distribution * (float)num_bits * probability);
   }
   
   if (num_crossovers > 0) { //  Do crossover
      cross_loc_bytes = (long *)malloc(num_crossovers * sizeof(long));
      if (cross_loc_bytes == (long *)NULL) return; //  Allocate the arrays for crossover locations
      cross_loc_bits = (long *)malloc(num_crossovers * sizeof(long));
      if (cross_loc_bits == (long *)NULL) return;
      
      range = (float) num_bits / ( (float) RAND_MAX + 1.0 ); //  Determine the crossover locations
      for (i=0; i<num_crossovers; i++) {
	 cross_loc_bits[i] = (long)((float)rand() * range);
	 cross_loc_bytes[i] = cross_loc_bits[i] / 8;
      }

      long offset = 0;
      for (i=0; i<num_crossovers; i++) { //  Set the source based on the crossover site
	 if ((i % 2) == 0) source = mother->GetGenome();
	 else source = father->GetGenome();
	 
	 cross_bit = LONG_MAX; //  Find the lowest crossover location (non-used)
	 for (j=0; j<num_crossovers; j++) {
	    if (cross_loc_bits[i] < cross_bit && cross_loc_bits[i] != 0) {
	       index = i;
	       cross_bit = cross_loc_bits[i];
	    }
	 }
	 if (cross_bit == LONG_MAX) break;
	 cross_byte = cross_loc_bytes[index];
	 cross_loc_bits[index] = 0;
	 
	 dest = data; //  If needed, cross the bits in the crossover byte
	 dest += cross_byte;
	 source += cross_byte;
	 offset += cross_byte;
	 cross_bit = cross_bit % 8;
	 if (cross_bit != 7) {
	    value = *source;
	    for (bit=7; bit>cross_bit; bit--) {
	       bit_mask = 1 << bit;
	       *dest &= ~bit_mask;
	       if (value >= bit_mask) {
		  *dest |= bit_mask;
		  value &= ~bit_mask;
	       }
	    }
	 }
	 
	 if (cross_byte + 1 < num_bytes) {
	    dest ++; //  If needed, copy the remaining bytes from the source
	    source ++;
	    offset ++;
	    //CopyFrom( neo->prob_insert_delete, mother, num_bytes - cross_byte - 1, offset );
	    memcpy(dest, source, (size_t)(num_bytes - cross_byte - 1));
	 }
      }
      free(cross_loc_bits);
      free(cross_loc_bytes);
   }
}

void Genome::Mutate(float probability) {
   long num_mutations, i, byte, bit;
   float distribution = 0.0, range;
   if (probability <= 0.0) return; //  Use the probability to determine the number of bits to mutate
   for (i=0; i<16; i++) distribution += (float)(rand() & 0x7fff);
   distribution *= fDistributionConstant;
   num_mutations = (long)((float)num_bits * probability * distribution + 0.5);
   if (num_mutations == 0) return;
   range = (float) num_bits / ( (float) RAND_MAX + 1.0 );

   for (i=0; i<num_mutations; i++) { //  Determine the bit for this mutation
      bit = (long)(range * (float)rand());
      byte = bit / 8;
      bit = bit % 8;
      data[byte] ^= 1 << bit; //  Swap the state of the selected bit
   }
}

void Genome::CopyFrom( Genome *source, long nbytes, long offset ) {
   if ( nbytes < 0 ) nbytes = source->GetByteLength();
   memcpy( data + offset, source->GetGenome() + offset, (size_t) nbytes );
}

#ifdef VARY_NNETS
void Genome::InsertDelete( float probability ) {
   if ( probability <= 0.0 ) return;
   float distribution = 0.0;
   for ( int i = 0; i < 16; i ++ ) distribution += (float)(rand() & 0x7fff);
   distribution *= fDistributionConstant;
   long num_mutations = (long)((float)num_bits * probability * distribution + 0.5);
   if ( num_mutations == 0 ) return;

   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   for ( int i = 0; i < num_mutations; i ++ ) { // Figure out whether to change sensors or neurons or
      int which = ( rand() & 0x7fff ) % 3; // outputs; figure out whether to increase or decrease them.
      int plusminus = ( rand() & 0x7fff ) % 2;
      //printf("%d %d\n",which,plusminus);
      if ( plusminus == 0 ) {
	 if ( which == 0 && numsensors < MAX_SENSORS ) AddSensor( rand() % numsensors );
	 else if ( which == 1 && numneurons < MAX_NEURONS ) AddNeuron( rand() % numneurons );
	 else if ( which == 2 && numoutputs < MAX_OUTPUTS ) AddOutput( rand() % numoutputs );
      } else {
	 if ( which == 0 && numsensors > MIN_SENSORS ) DeleteSensor( rand() % numsensors );
	 else if ( which == 1 && numneurons > MIN_NEURONS ) DeleteNeuron( rand() % numneurons );
	 else if ( which == 2 && numoutputs > MIN_OUTPUTS ) DeleteOutput( rand() % numoutputs );
      }
   }
}

void Genome::AddSensor( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors+1, numneurons, numoutputs );
   memcpy( tmp->data, data, NUM_OTHER_BYTES );
   int i, j, k;
   for ( i = 0, j = 0; i < numsensors; i ++, j ++ ) {
      if ( i == index ) tmp->sensors[j ++] = rand() & 0x7fff;
      tmp->sensors[j] = sensors[i];
   }
   for ( k = 0; k < numneurons; k ++ ) {
      for ( i = 0, j = 0; i < numsensors; i ++, j ++ ) {
	 if ( i == index ) tmp->input_wgts[k + (j++)*numneurons] = rand() & 0x7fff;
	 tmp->input_wgts[k + j*numneurons] = input_wgts[k + i*numneurons];
      }
   }
   memcpy( tmp->intermed_thresh, intermed_thresh, 
   	   (long) numneurons + (long) numneurons * (long) numoutputs );
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}

void Genome::AddNeuron( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors, numneurons+1, numoutputs );
   memcpy( tmp->data, data, NUM_OTHER_BYTES );
   memcpy( tmp->sensors, sensors, numsensors+1 );
   int i, j, k;
   for ( i = 0; i < numsensors; i ++ ) {
      for ( k = 0, j = 0; k < numneurons; k ++, j ++ ) {
	 if ( k == index ) tmp->input_wgts[j++ + i*(numneurons+1)] = rand() & 0x7fff;
	 tmp->input_wgts[j + i*(numneurons+1)] = input_wgts[k + i*numneurons];
      }
   }
   for ( i = 0, j = 0; i < numneurons; i ++, j ++ ) {
      if ( i == index ) tmp->intermed_thresh[j ++] = rand() & 0x7fff;
      tmp->intermed_thresh[j] = intermed_thresh[i];
   }
   for ( i = 0; i < numoutputs; i ++ ) {
      for ( k = 0, j = 0; k < numneurons; k ++, j ++ ) {
	 if ( k == index ) tmp->output_wgts[j++ + i*(numneurons+1)] = rand() & 0x7fff;
	 tmp->output_wgts[j + i*(numneurons+1)] = output_wgts[k + i*numneurons];
      }
   }
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}

void Genome::AddOutput( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors, numneurons, numoutputs+1 );
   memcpy( tmp->data, data, (output_wgts - (signed char *) data) + 1 );
   int i, j, k;
   for ( i = 0; i < numneurons; i ++ ) {
      for ( k = 0, j = 0; k < numoutputs; k ++, j ++ ) {
	 if ( k == index ) tmp->output_wgts[i + (j++)*numneurons] = rand() & 0x7fff;
	 tmp->output_wgts[i + j*numneurons] = output_wgts[i + k*numneurons];
      }
   }
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}

void Genome::DeleteSensor( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors-1, numneurons, numoutputs );
   memcpy( tmp->data, data, NUM_OTHER_BYTES );
   int i, j, k;
   for ( i = 0, j = 0; j < numsensors-1; i ++, j ++ ) {
      if ( j == index ) i ++;
      tmp->sensors[j] = sensors[i];
   }
   for ( k = 0; k < numneurons; k ++ ) {
      for ( i = 0, j = 0; j < numsensors; i ++, j ++ ) {
	 if ( j == index ) i ++;
	 tmp->input_wgts[k + j*numneurons] = input_wgts[k + i*numneurons];
      }
   }
   memcpy( tmp->intermed_thresh, intermed_thresh, 
   	   (long) numneurons + (long) numneurons * (long) numoutputs );
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}

void Genome::DeleteNeuron( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors, numneurons-1, numoutputs );
   memcpy( tmp->data, data, NUM_OTHER_BYTES );
   memcpy( tmp->sensors, sensors, numsensors );
   int i, j, k;
   for ( i = 0; i < numsensors; i ++ ) {
      for ( k = 0, j = 0; j < numneurons - 1; k ++, j ++ ) {
	 if ( j == index ) k ++;
	 tmp->input_wgts[j + i*(numneurons-1)] = input_wgts[k + i*numneurons];
      }
   }
   for ( i = 0, j = 0; j < numneurons - 1; i ++, j ++ ) {
      if ( j == index ) i ++;
      tmp->intermed_thresh[j] = intermed_thresh[i];
   }
   for ( i = 0; i < numoutputs; i ++ ) {
      for ( k = 0, j = 0; j < numneurons - 1; k ++, j ++ ) {
	 if ( j == index ) k ++;
	 tmp->output_wgts[j + i*(numneurons-1)] = output_wgts[k + i*numneurons];
      }
   }
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}

void Genome::DeleteOutput( int index ) {
   int numsensors = NumSensors(), numneurons = NumNeurons(), numoutputs = NumOutputs();
   Genome *tmp = new Genome( neo, numsensors, numneurons, numoutputs-1 );
   memcpy( tmp->data, data, (output_wgts - (signed char *) data) + 1 );
   int i, j, k;
   for ( i = 0; i < numneurons; i ++ ) {
      for ( k = 0, j = 0; j < numoutputs-1; k ++, j ++ ) {
	 if ( j == index ) k ++;
	 tmp->output_wgts[i + j*numneurons] = output_wgts[i + k*numneurons];
      }
   }
   Initialize( tmp->NumSensors(), tmp->NumNeurons(), tmp->NumOutputs() );
   memcpy( data, tmp->data, num_bytes );
   delete tmp;
}
#endif

float Genome::GetDistributionConstant() {
   long max = RAND_MAX & 0x7fff;
   max *= 16;
   return 1.0 / (float) max;
}

void Genome::GetSensorString( unsigned char sensor, char *string ) {
   unsigned char temp = sensor & 31; // First 5 bits of sensor
   if ( temp < 25 ) { // Directional sensor
      long x, y, xoffset, yoffset;
      xoffset = ( temp % 5 ) - 2;
      yoffset = ( temp / 5 ) - 2;
      temp = ( sensor >> 5 ) & 7; // Type of sensor is in last 3 bits
      switch ( temp ) {
      case 0: case 1: ::sprintf( string, "Fo:%d,%d", xoffset, yoffset ); break;
      case 2: 
#ifndef USE_BARRIER
      case 3: ::sprintf( string, "Po:%d,%d", xoffset, yoffset ); break;
#else
      case 3: ::sprintf( string, "B:%d,%d", xoffset, yoffset ); break;
#endif
      case 4: ::sprintf( string, "C:%d,%d", xoffset, yoffset ); break;
      case 5: ::sprintf( string, "C1:%d,%d", xoffset, yoffset ); break;
      case 6: ::sprintf( string, "C2:%d,%d", xoffset, yoffset ); break;
      case 7: ::sprintf( string, "Cb:%d,%d", xoffset, yoffset ); break;
      }
   } else {
      temp = ( sensor >> 5 ) & 7;
      temp = temp % 7;
      switch ( temp ) {
      case 0: ::strcpy( string, "En" ); break;
      case 1: ::strcpy( string, "Act" ); break;
      case 2: ::strcpy( string, "Age" ); break;
      case 3: ::strcpy( string, "Dir" ); break;	 
      case 4: ::strcpy( string, "X" ); break;
      case 5: ::strcpy( string, "Y" ); break;
      case 6: ::strcpy( string, "Mass" ); break;
      case 7: ::strcpy( string, "Ran" ); break;
      }
   }
}

void Genome::Output() {
   unsigned char NUMNEURONS = NumNeurons(), NUMSENSORS = NumSensors(), NUMOUTPUTS = NumOutputs();
   printf( "Metabolism = %d; Max. Mass = %d; Food Type = %d\n", (int) *metabolism, 
	   (int) *maximum_mass, (int) *plant_eff );
   char entry[64];
   printf( "Sensors: \n" );
   for (int i=0; i<NUMSENSORS; i++) {
      GetSensorString( sensors[i], entry );
      printf( "%s\t", entry );
   }
   printf( "\n" );
   printf( "Input-hidden Weights:\n" ); //  Next the input-hidden weights
   for (int i=0; i<NUMNEURONS; i++) {
      for (int j=0; j<NUMSENSORS; j++) {
	 //::sprintf(entry, "%d,%d=%4d", i, j, (int)input_wgts[i + j*NUMNEURONS]);
	 ::sprintf(entry, "%4d", (int)input_wgts[i + j*NUMNEURONS]);
	 printf( "%s ", entry );
      }
      printf( "\n" );
   }
   printf( "Hidden Thresholds:\n" ); //  Now, the hidden thresholds
   for (int i=0; i<NUMNEURONS; i++) {
      //::sprintf(entry, "%d=%4d", i, (int)intermed_thresh[i] << 4);
      ::sprintf(entry, "%4d", (int)intermed_thresh[i] << 4);
      printf( "%s ", entry );
   }
   printf( "\n" );
   printf( "Output Weights:\n" ); //  finally, the output weights
   for (int i=0; i<NUMOUTPUTS; i++) {
      for (int j=0; j<NUMNEURONS; j++) {
	 //::sprintf(entry, "%d,%d=%4d", i, j, (int)output_wgts[i*NUMNEURONS + j]);
	 ::sprintf(entry, "%4d", (int)output_wgts[i*NUMNEURONS + j]);
	 printf( "%s ", entry );
      }
      printf( "\n" );
   }
}

void Genome::WriteToFile( FILE *file ) {
   fwrite( &num_sensors_outputs, sizeof( num_sensors_outputs ), 1, file );
   fwrite( &num_neurons, sizeof( num_neurons ), 1, file );
   fwrite( data, num_bytes, 1, file );
}

void Genome::ReadFromFile( FILE *file ) {
   fread( &num_sensors_outputs, sizeof( num_sensors_outputs ), 1, file );
   fread( &num_neurons, sizeof( num_neurons ), 1, file );
   Initialize( NumSensors(), NumNeurons(), NumOutputs() );
   fread( data, num_bytes, 1, file );
}
