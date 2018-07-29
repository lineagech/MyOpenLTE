#define pi 3.1415926
// AWGN generation using Box-Muller transform
void AWGN(float *data_I, float *data_Q, unsigned long long arraylength, float var){
    float n1,n2;
    float maximum = (float)RAND_MAX;
    
    //srand(time(NULL));
    
    // �̷ӨC���q�D��q���j�p�[noise �}�l //
    float power_RX;
    power_RX=0;
    for(unsigned long long i=0; i<arraylength; i++){
        power_RX+=data_I[i]*data_I[i]+data_Q[i]*data_Q[i];
    } 
    power_RX/=(float)arraylength;  //�����C��sample����q
    power_RX=power_RX*var/2.0; 
    // �̷ӨC���q�D��q���j�p�[noise ���� // 

    for(unsigned long long i=0; i<arraylength; i++){
       n1 = (float)rand()/maximum;
       n2 = (float)rand()/maximum;
    ////////////////////////////////////////////////   
       while(n1==0){
         n1 = (float)rand()/maximum;                      
       }             
       while(n2==0){
         n2 = (float)rand()/maximum;                    
       }        
    ////////////////////////////////////////////////      
          
       //data_I[i] += sqrt(-2*log(n1)*var)*cos(2*pi*n2);   //�T���[�WAWGN  additive white gaussian noise
       //data_Q[i] += sqrt(-2*log(n1)*var)*sin(2*pi*n2);   //sqrt(var)�Y�зǮt
        
       // �̷ӨC���q�D��q���j�p�[noise �}�l //
       data_I[i] += sqrt(-2*log(n1)*power_RX)*cos(2*pi*n2);
       data_Q[i] += sqrt(-2*log(n1)*power_RX)*sin(2*pi*n2);
       // �̷ӨC���q�D��q���j�p�[noise ���� // 
    }
}

/***********************************************************************************/
// Carrier Frequency Offset
void CFO(float* CFO_I,float* CFO_Q,float FO,float PO,unsigned long long seq_length)    //���B��seq_length�N��Y�@�ѽu������`����
{
	  unsigned long long i;
	  float a,b;
	  FO=FO/pow(10,6);
	  for(i=0;i<seq_length;i++)                        //�������B��i��accumulater�ҥN���N�t
	  {
   
		a=CFO_I[i];
		b=CFO_Q[i];
		/* 
	    CFO_I[i] = a*cos(2*pi*FO*i+PO)+b*sin(2*pi*FO*i+PO);   //����accumulater�Pi�P�ɼW�[  �i�H��accumulater����FO  ?
		CFO_Q[i] = b*cos(2*pi*FO*i+PO)-a*sin(2*pi*FO*i+PO);
		*/ 
		
		
		//�ץ�CFO�������~ 
		
	    CFO_I[i] = a*cos(2*pi*FO*i+PO)-b*sin(2*pi*FO*i+PO);   //����accumulater�Pi�P�ɼW�[  �i�H��accumulater����FO  ?
		CFO_Q[i] = b*cos(2*pi*FO*i+PO)+a*sin(2*pi*FO*i+PO);		
		
		//�n�� 
		/*
	    CFO_I[i] = a*cos(2*pi*FO/seq_length*i*i+PO)-b*sin(2*pi*FO/seq_length*i*i+PO);   //����accumulater�Pi�P�ɼW�[  �i�H��accumulater����FO  ?
		CFO_Q[i] = b*cos(2*pi*FO/seq_length*i*i+PO)+a*sin(2*pi*FO/seq_length*i*i+PO);
        */			
	  }       
}

/***********************************************************************************/
// Phase Noise
void PN(float* nI,float* nQ,float p_noise,unsigned long long seq_length)    //p_noise��variance?  ==>sqrt(p_noise)�Ystandard deviation
{    	     			     
    float n1,n2;
	float phi=0,phq=0;
	float a,b;
    float maximum = (float)RAND_MAX;
    srand(time(NULL));
    for(unsigned long long i=0; i<seq_length; i++){                              //���B��seq_length�N��Y�@�ѽu������`����
       n1 = (float)rand()/maximum;
       n2 = (float)rand()/maximum;
    ////////////////////////////////////////////////   
       while(n1==0){
         n1 = (float)rand()/maximum;                      
       }             
       while(n2==0){
         n2 = (float)rand()/maximum;                    
       }        
    ////////////////////////////////////////////////  
       phi += sqrt(-2*log(n1)*p_noise)*cos(2*pi*n2);      //phase noise�OGaussian random variable�����_�ۥ[
       phq += sqrt(-2*log(n1)*p_noise)*sin(2*pi*n2);      //�]��phq�O�h�l��?
       a=nI[i];
	   b=nQ[i];
       nI[i] = a*cos(phi)-b*sin(phi); 
       nQ[i] = b*cos(phi)+a*sin(phi);                      //���ӬOb*cos(phi)+a*sin(phi)  ??
    }
}

/***********************************************************************************/
// IQ imbalance
void IQ_Imbalance(float* IQ_I,float* IQ_Q,float alpha,float phase,unsigned long long seq_length)
{
   unsigned long long i;
   float a,b;
   for(i=0;i<seq_length;i++)
   {
    a=IQ_I[i];
    b=IQ_Q[i];	
    IQ_I[i] = (1+alpha)*(a*cos(phase/2)-b*sin(phase/2));           //�䤤carrier frequency offset�������]���g�b�t�~�Ƶ{��(CFO)�G�S����J
    IQ_Q[i] = (1-alpha)*(b*sin(-phase/2)+b*cos(-phase/2));         //�����N�����Y�i
   }
}

/***********************************************************************************/
// DC offset
void DCoffset(float* DC_I,float* DC_Q,float I_DC,float Q_DC, unsigned long long seq_length)
{
	unsigned long long i;
	for(i=0;i<seq_length;i++)
	{
      DC_I[i]=DC_I[i]+I_DC;                                          //�����[�Woffset
      DC_Q[i]=DC_Q[i]+Q_DC;
	}
}

/***********************************************************************************/
// Sampling Clock Offset
void SCO(float *SCO_out,float* SCO_in,float offset, float initial,unsigned long long seq_length)
{
	//Farrow interpolator
	unsigned long long m;
	float mu;
	unsigned long long k;
    offset=offset/pow(10,6);
  for(k=0;k<seq_length;k++)
  {

 	m =(unsigned long long)( k*(1.0+offset)+initial);   //���ק�                //1+offset=Tnew/Torig     �]��m�O��ƫ����A�ҥH���floor  �ڧ令(int) 
     


	mu = k*(1.0+offset)+initial-m;       //���ק�  

	//initial conditions
	if(k==0)
	{              
	  SCO_out[k] =   SCO_in[m+2]*(-0.5*mu+0.5*mu*mu)
	    	        +SCO_in[m+1]*(1.5*mu-0.5*mu*mu)
                    +SCO_in[m]*(1-0.5*mu-0.5*mu*mu);
                                      //���ק�        //����ݭninitial   �Ĥ@����ƪ���X���G�ä���������initial offset
	}
	
	else
	{
	  if( (m+2)<= seq_length-1 )         //�P�_m+2,m+1,m,m-1�O�_�W�X��ƦC��index,�Y�W�X�����ݭק�
	  {
		
		SCO_out[k] =   SCO_in[m+2]*(-0.5*mu+0.5*mu*mu)
	    	        +SCO_in[m+1]*(1.5*mu-0.5*mu*mu)
                    +SCO_in[m]*(1-0.5*mu-0.5*mu*mu)
                    +SCO_in[m-1]*(-0.5*mu+0.5*mu*mu);
	  }
	  else
	  {
		  if( (m+1)<= seq_length-1 )
		  {
               SCO_out[k] =  SCO_in[m+1]*(1.5*mu-0.5*mu*mu)
                    +SCO_in[m]*(1-0.5*mu-0.5*mu*mu)
                    +SCO_in[m-1]*(-0.5*mu+0.5*mu*mu);
		  }
		  else
		  {
               if( m<= seq_length-1 )
		      {
               SCO_out[k] =  SCO_in[m]*(1-0.5*mu-0.5*mu*mu)
                    +SCO_in[m-1]*(-0.5*mu+0.5*mu*mu);
		      }
			   else
			   {
				   if( (m-1)<= seq_length-1 )
		          {
                    SCO_out[k] = SCO_in[m-1]*(-0.5*mu+0.5*mu*mu);
		          }
				   else
				   {
                       SCO_out[k]=SCO_out[k-1];
				   }//end if
			   }//end if 
		  }//end if
	  }//end if

	}//end if
  }//end for k
}//end void
