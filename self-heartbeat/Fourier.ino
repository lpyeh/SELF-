/** This function contains all the code required to calculate the stress levels from the readings. */

float LFHF_max_current = 0.0;
float LFHF_min_current = 10.0;
float LFHF_avg = 0.0;
// start from "neutral"
// a lower score is "better"/"low stress"
// a higher score is "high stress"
int user_score = 0;

void run_Fourier() {
  float sum = 0.0;
  int iter = 0;
  
  // start while/for
  // while curr_min < 5 min
  LFHF = Fourier();
  sum += LFHF;
  iter++;
  
  if (LFHF > LFHF_max_current) {
      LFHF_max_current = LFHF;
  }
  else if (LFHF < LFHF_min_current) {
      LFHF_min_current = LFHF;
  } // end while/for

  LFHF_avg = float(sum / iter);
}

void fourier_score() {
  // could also include athlete information 
  if (!sex) {
    user_score = score_female();
  }
  else {
    user_score = score_male();
  }
  feedback();
}

float Fourier(){ 
 
 // Check if we have enough data for Fourier transform (lower than 31 means not enough) 
 // if not enough, add current reading to the array
 if (dataCount < 31){
    if (dataCount == 0) { Serial.print("Waiting to gather enough data points before running Fourier transform"); }
    dataCount++;
    data[dataCount] = IBI;
  }
 // If enough data is gathered, continue with calculation
  else{        
  	for (int i = 0 ; i < FHT_N-1 ; i++) { // save 256 samples
  		data[i] = data[i+1];
  	}
			// add current reading to the list
   data[31]=IBI;
    
   for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      fht_input[i] = data[i]; // put real data into bins
    }
    
    noInterrupts();
    fht_window(); 	// window the data for better frequency response
    fht_reorder(); 	// reorder the data before doing the fht
    fht_run(); 		// process the data in the fht
    fht_mag_log(); 	// get the output of the fht
    interrupts();

    // not sure if this is actually total power spectral density 
    for (int i = 0; i < 12; i++) {
      P += fht_log_out[a];
    }
      
    for (int a = 2; a < 5; a++){
      LF += fht_log_out[a];
    }
    
    for (int a = 6; a < 12; a++){
      HF += fht_log_out[a];
    }
    LF=LF/3;
    HF=HF/6;
    LFHF=LF/HF;
    
    Serial.print("LFHF ");
    Serial.println(LFHF);

    vibeTimeSet = 500*((LFHFOld + (LFHF))/2);
    LFHFOld = LFHF;
    
    return LFHF;
  }
}

int score_female() {
  int score = 0;
  // young female
  if (age <= 49 && age >= 25) {
    // if average is lower than average, lower the score
    (LFHF_avg <= YF_avg) ? score-- : score++;
    (LFHF_min <= YF_min) ? score-- : score++;
    (LFHF_max <= YF_max) ? score-- score++;
  }
  // elderly female
  else {
    (LFHF_avg <= EF_avg) ? score-- : score++;
    (LFHF_min <= EF_min) ? score-- : score++;
    (LFHF_max <= EF_max) ? score-- : score++;
  }
}

int score_male() {
  int score = 0;
  // young male
  if (age <= 49 && age >= 25) {
    (LFHF_avg <= YM_avg) ? score-- : score++;
    (LFHF_min <= YM_min) ? score-- : score++;
    (LFHF_max <= YM_max) ? score-- score++;
  }
  else {
    (LFHF_avg <= EM_avg) ? score-- : score++;
    (LFHF_min <= EM_min) ? score-- : score++;
    (LFHF_max <= EM_max) ? score-- : score++;
  }
}

void feedback() {
  
}
