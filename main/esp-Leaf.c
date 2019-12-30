/* this my thesis code.... 
Author: Caleb Seifert
	And example code available in the Espressif IDF/ ESP-Face Library
       
   This code is in the Public Domain (or CC0 licensed, at your option.)
 ** I DONT KNOW HOW TO LICENSE THIS CODE, 
 	IF YOU USE IT PLEASE JUST LINK TO MY GITHUB SOMEWHERE**

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

	        III
   		III		

	  HHHH	III  HHH
	  HHHH	III  HHH
	  HHHHHHIIIHHHHH
	  HHHHHHIIIHHHHH
	  HHHH	III  HHH
	  HHHH	III  HHH
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"// CORE 2 stuff
#include "freertos/task.h"// multi threaded lib

#include "esp_system.h"
//#include "esp_spi_flash.h"//SD card, use later..
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"
#include "esp_log.h"



//#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"  
#include "image_util.h"
#include "driver/rtc_io.h"

//#include "esp32-hal-psram.h"  


//#include "esp_err.h"
//#include "driver/ledc.h"
#include "fd_forward.h" //Espressif MTCNN Lib
//#include "mtmn.h"
#include "fr_forward.h"
#include "fb_gfx.h"
#include "dl_lib_matrix3d.h"

//weights from new net
#include "pnetWeights.h"
#include "rnetCut16Weights.h"
//#include "onetCut4Weights.h"
#include "sdkconfig.h"



#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
/*
#include "esp_http_server.h"
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
*/



#define EEPROM_SIZE 1
// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13
//
//
//static xSemaphoreHandle seminoleKey;// = NULL;
//incaKey
//mayanKey (skeletonKey)
xSemaphoreHandle skeletonKey;

   static mtmn_net_t outHold={0};// {mtmn_net_t};
		  //  {.category=NULL//dl_matrix3d_alloc(1,1,1,1) 
	   	  // , .offset=NULL};//dl_matrix3d_alloc(1,1,1,1)};
mtmn_net_t *output = (mtmn_net_t*)&outHold;

static net_config_t confSpace={0};
	net_config_t *currentConfig = (net_config_t*)&confSpace;
//

void initOut(int wS,int hS,int wB,int hB)
{

       output->category=dl_matrix3d_alloc(1,wS,hS,2);
       output->offset=dl_matrix3d_alloc(1,wB,hB,4);// (mtmn_net_t*)&outHold;
}



typedef struct {     
   
     	size_t len;

} jpg_chunking_t;



static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
  



    j->len += len;
    return len;
}
 

bool cameraConfig()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

   config.pixel_format = PIXFORMAT_JPEG;//GRAYSCALE;//JPEG; 

    config.frame_size = FRAMESIZE_QVGA;
    // FRAMESIZE_ + QVGA (160X120) |CIF (400X296)
    // |VGA (640X480) |SVGA (800X600) |XGA (1024X768)
    // |SXGA (1280X1024)|UXGA (1600X1200)
    config.jpeg_quality = 10;
    config.fb_count =3;//2;//3;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    printf("Camera init failed with error");
    return false;
  }
  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);//FRAMESIZE_QVGA); 
  s->set_brightness(s,1);
  return true;
}

 
void saveImage(camera_fb_t *in,int id)//dl_matrix3du_t *in,  char* fileName)

{
   size_t _jpg_buf_len = 0;
   uint8_t * _jpg_buf = NULL;
//camera_fb_t * out = NULL;

int size = in->len;
printf("[SD] size: %i\n",size);

sdmmc_host_t host = SDMMC_HOST_DEFAULT();
 sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
 gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
 gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
 gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
 gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
 gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

//gpio_pulldown_dis(13);
//gpio_pullup_en(13);

esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size =16* 1024//size//img->w*img->c  // 16 * 1024

    };

    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

vTaskDelay(1000/portTICK_PERIOD_MS);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf( "[SD] Failed to mount filesystem.");
        } else {
            printf( "[SD] Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    sdmmc_card_print_info(stdout, card);
uint32_t free1 = esp_get_free_heap_size();
printf("\n[SD] FREE HEAP BEFORE OPENING FILE: %i \n",free1);
uint32_t free8Bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
uint32_t freeIn = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
printf("[SD] FREE CAP 8 BIT: %i \n",free8Bit);
printf("[SD] FREE INTERNAL: %i \n",freeIn);

 	
printf("\n[SD] OPENING FILE\n");
    	FILE* f = fopen("/sdcard/meh.jpg", "w");

printf("\n[SD] FILE OPENED ...size: %i \n",size);

	if (f == NULL)
       	{
		perror("/sdcard/meh.jpg");
        	printf( "[SD] Failed to open file for writing");
        	return;
	}

       //=====================
      //write the image    
 	size_t fb_len = 0;
        if (in->format == PIXFORMAT_JPEG)
        {   printf("\nJPEG FORMAT! LETS TRY WRITING\n");
            fb_len = in->len;
	    printf("SIZE: %i inside: %i  \n",size,in->len);
            fwrite((const char *)in->buf,sizeof(char *),in->len,f);
	    //use size instead of in->len if not good
        }
        else
        {
          printf("\n WRONG FORMAT!\n");
	  fclose(f);
	   esp_vfs_fat_sdmmc_unmount();
	  return;
        };
    //fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    printf("\nFile written\n");
     
 esp_vfs_fat_sdmmc_unmount();
}



void setOutput(dl_matrix3d_t *finalLayer,int id)
{
        if(id==0)
        {
          output->category=finalLayer;
	}
	else  if(id==1)
        {
          output->offset=finalLayer;
    	}
	else if(id == 3)
	{
          printf("\n[OUTPUT] 死了\n");
	}
        else
        {
        printf("\n\nCANT SET OUTPUT!\n\n");
        //offset=NULL;
        }
}


mtmn_net_t getOutput()
{
 return *output;
}


void pnetCore1(void *pvParameter)
{
  printf("\n[PNET] HELLO P net!\n");	

  while(true)
  {	      
	xSemaphoreTake(skeletonKey,portMAX_DELAY);  
        //=====================================
        // PNET LAYER 1

	dl_matrix3d_t *in =(dl_matrix3d_t*) pvParameter;
  	dl_matrix3d_t *filt_c1 = dl_matrix3d_alloc(10,3,3,3);//(10,3,3,3);
	dl_matrix3d_t *bias1   = dl_matrix3d_alloc(1,1,1,10);//(1,1,1,10);	
	dl_matrix3d_t *prelu1  = dl_matrix3d_alloc(1,1,1,10);//(1,1,1,10); 
	dl_matrix3d_t *out1;   
	dl_matrix3d_t *outPool;

  //init weights
  getpnet_conv1_0(filt_c1);//pnetVals.h
  getpnet_conv1_1(bias1);//pnetvals.h
  getpnet_prelu1_0(prelu1);//pnetvals.h
	//calculate
	out1= dl_matrix3dff_conv_3x3(in,filt_c1,bias1,1,1,PADDING_VALID);
	//clean up
	dl_matrix3d_free(filt_c1);
	dl_matrix3d_free(bias1);
	//pool
   	outPool=dl_matrix3d_pooling(out1,1,1,2,2,PADDING_VALID,DL_POOLING_MAX); 
	//clean up
	dl_matrix3d_free(out1);
	dl_matrix3d_p_relu(outPool, prelu1);
	dl_matrix3d_free(prelu1);

        //=====================================
        // PNET LAYER 2
	dl_matrix3d_t *filt_c2 = dl_matrix3d_alloc(16,3,3,10);
	dl_matrix3d_t *bias2   = dl_matrix3d_alloc(1,1,1,16);
	dl_matrix3d_t *pool2;
	dl_matrix3d_t *out2;
	dl_matrix3d_t *prelu2 = dl_matrix3d_alloc(1,1,1,16);
  //init weights
  getpnet_conv2_0(filt_c2);
  getpnet_conv2_1(bias2);
  getpnet_prelu2_0(prelu2);
	//calculate
	out2 = dl_matrix3dff_conv_3x3(outPool,filt_c2,bias2,1,1,PADDING_VALID);
	//clean up
	dl_matrix3d_free(outPool);
	//pool
	pool2=dl_matrix3d_pooling(out2,1,1,2,2,PADDING_VALID,DL_POOLING_MAX);
	//clean up
	dl_matrix3d_free(out2);
	dl_matrix3d_p_relu(pool2,prelu2);
	dl_matrix3d_free(prelu2);
	
//printf("\n[PNET] OUT FROM POOL 2:\n");
//	for(int i=0;i<16;i++)
//		 printf("##  %f ##\n",pool2->item[i]);
		
	//=====================================
	// PNET LAYER 3
	dl_matrix3d_t *filt_c3 = dl_matrix3d_alloc(32,3,3,16);
        dl_matrix3d_t *bias3   = dl_matrix3d_alloc(1,1,1,32);
	dl_matrix3d_t *pool3;
        dl_matrix3d_t *out3;
	//get weights
        getpnet_conv3_0(filt_c3);
        getpnet_conv3_1(bias3);
	//calculate
        out3 = dl_matrix3dff_conv_3x3(pool2,filt_c3,bias3,1,1,PADDING_VALID);
	//clean up
	dl_matrix3d_free(pool2);
	dl_matrix3d_free(bias3);
	dl_matrix3d_free(filt_c3);
	//pool
	pool3=dl_matrix3d_pooling(out3,1,1,2,2,PADDING_VALID,DL_POOLING_MAX);
	//clean up
	dl_matrix3d_free(out3);
	dl_matrix3d_t *prelu3 = dl_matrix3d_alloc(1,1,1,32);
        getpnet_prelu3_0(prelu3);
        //prelu
	dl_matrix3d_p_relu(pool3,prelu3);                
	dl_matrix3d_free(prelu3);

//printf("\n[PNET] OUT FROM POOL 3:\n");
//        for(int i=0;i<32;i++)
//                 printf("##  %f ##\n",pool3->item[i]);
	
	 //====================================
        // Scores
	dl_matrix3d_t *score_filter=dl_matrix3d_alloc(2,1,1,32);
	dl_matrix3d_t *score_bias = dl_matrix3d_alloc(1,1,1,2);
	dl_matrix3d_t *score_out;

	getpnet_score_0(score_filter);
	getpnet_score_1(score_bias);

score_out= dl_matrix3dff_conv_3x3(pool3,score_filter,score_bias,1,1,PADDING_VALID);


	dl_matrix3d_free(score_filter);
	dl_matrix3d_free(score_bias);
	dl_matrix3d_softmax(score_out);	
//      for(int i=0;i<20;i++)
//                printf("\n  %f ",score_out->item[i]);
//printf("\n[PNET] SHAPE of score out  %i %i %i %i\n",score_out->n,score_out->w,score_out->h,score_out->c);

	//====================================
	// Bounding Boxes
	dl_matrix3d_t *bbox_filter=dl_matrix3d_alloc(4,1,1,32);
	dl_matrix3d_t *bbox_bias=dl_matrix3d_alloc(1,1,1,4);
	dl_matrix3d_t *bbox_out=dl_matrix3d_alloc(1,1,1,4);
	
	getpnet_bbox_pred_0(bbox_filter);
	getpnet_bbox_pred_1(bbox_bias);

bbox_out = dl_matrix3dff_conv_3x3(pool3,bbox_filter,bbox_bias,1,1,PADDING_VALID);
	//dl_matrix3d_free(out3);
	dl_matrix3d_free(bbox_filter);
	dl_matrix3d_free(bbox_bias);
	dl_matrix3d_free(pool3);
//=========================================
//		SET MEMORY		 //

	//output->offset=bbox_out;

//      for(int i=0;i<20;i++)
//                printf("\n  %f ",score_out->item[i]);
	
	initOut(score_out->w,score_out->h,bbox_out->w,bbox_out->h );
	setOutput(bbox_out,1);// bounding boxes
	setOutput(score_out,0);//score output	

//printf("\n\n");
 
        dl_matrix3d_free(bbox_out);
        dl_matrix3d_free(score_out);
printf("\n[PNET] Bye PNET!\n");
xSemaphoreGive(skeletonKey);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelete( NULL );
 }//endForeverWhile

printf("\n\n\n[PNET] I SHOULD NOT SEE THIS! \n\n\n");
//vTaskDelay(1000/portTICK_PERIOD_MS);
for(; ; )
{
      vTaskDelay(1000/portTICK_PERIOD_MS);
      xSemaphoreGive(skeletonKey);
      printf(".");     
	vTaskDelete( NULL );
}

}


void rnetCore1(void  *pvParam)//mtmn_net_t *in)
{ 
 for( ; ; )
 {
	dl_matrix3d_t *in =(dl_matrix3d_t*) pvParam;
 	xSemaphoreTake(skeletonKey,portMAX_DELAY); 
	//input -> shape (48,48, RGB) 
	
	//=================================
	// RNET LAYER 1
printf("\n\n\n\nWELCOME TO RNET!\n\n\n\n");
        
	dl_matrix3d_t *filt_c1 = dl_matrix3d_alloc(28,3,3,3);
        dl_matrix3d_t *bias1 = dl_matrix3d_alloc(1,1,1,28);
        dl_matrix3d_t *out1;//out should be (1,1,1,28);
        dl_matrix3d_t *pool1;
        dl_matrix3d_t *prelu1 = dl_matrix3d_alloc(1,1,1,28);
//printf("\n[RNET] INIT CONV 1!\n\n");
	getrnet_conv1_0(filt_c1);
        getrnet_conv1_1(bias1);
        getrnet_prelu1_0(prelu1);
//printf("\n\nconv 1 starting...\n\n");
	out1 = dl_matrix3dff_conv_3x3(in,filt_c1,bias1,1,1,PADDING_SAME);
	dl_matrix3d_free(in);
	dl_matrix3d_free(bias1);
	dl_matrix3d_free(filt_c1);
//printf("\nDONE CONV 1!!\n");
//printf("\n[RNET] SHAPE of out1 N: %i  W: %i  H:%i  C: %i\n",out1->n,out1->w,out1->h,out1->c);

	dl_matrix3d_p_relu(out1,prelu1);
//printf("\nPRELU 1 Done!\n");
	dl_matrix3d_free(prelu1);
	pool1=dl_matrix3d_pooling(out1,1,1,2,2,PADDING_VALID,DL_POOLING_MAX);
	dl_matrix3d_free(out1);
//  printf("\n[RNET] SHAPE of pool1 N: %i  W: %i  H:%i  C: %i\n",pool1->n,pool1->w,pool1->h,pool1->c);

//printf("\n\n[RNET] global pool 1 done!!\n\n");
        //=====================================
	// Rnet LAYER 2 
        //(48,3,3,28);

      //init
      dl_matrix3d_t *fconv2_1 = dl_matrix3d_alloc(24,3,3,28); 
      dl_matrix3d_t *fconv2_2 = dl_matrix3d_alloc(24,3,3,28); 
      dl_matrix3d_t *bias2 = dl_matrix3d_alloc(1,1,1,48);

        getrnet_cut0_conv2_0(fconv2_1);
        getrnet_cut1_conv2_0(fconv2_2);
       	getrnet_conv2_1(bias2);

//printf("\n\n[RNET] conv 2 starting...\n\n");
	//==========
	//conv split 
	
	//2 for in (pool shape: (1,12,12,28) ) *0.5 = (1,12,12,14) 
	
	dl_matrix3d_t *pool1_1 = dl_matrix3d_alloc(1,6,6,28);
	dl_matrix3d_t *pool1_2 = dl_matrix3d_alloc(1,6,6,28);

       	int px = (pool1->h)/2;
	int py = (pool1->w)/2;
	int pw = pool1_1->w;
	int ph = pool1_1->h;	

	dl_matrix3d_slice_copy(pool1_1, pool1, 0, 0, pw, ph);
        dl_matrix3d_slice_copy(pool1_2, pool1, px, py, pw, ph);

	//2 for bias
	dl_matrix3d_t *bias2_1 = dl_matrix3d_alloc(1,1,1,24);
        dl_matrix3d_t *bias2_2 = dl_matrix3d_alloc(1,1,1,24);
	int n = ((bias2->c)/2);//*pool1->stride;
       for(int i=0;i<n;i++)
                bias2_1->item[i] = bias2->item[i];	
	for(int i=n;i<(n*2);i++)
                bias2_2->item[i-n] = bias2->item[i];
	dl_matrix3d_free(bias2);

	//use 2 for filter done earlier
        dl_matrix3d_t *out2_1;
        dl_matrix3d_t *out2_2;

out2_1=dl_matrix3dff_conv_3x3(pool1_1,fconv2_1,bias2_1,1,1,PADDING_VALID);
// printf("\n[RNET] SHAPE of out2_1 N: %i  W: %i  H:%i  C: %i\n",out2_1->n,out2_1->w,out2_1->h,out2_1->c);

out2_2=	dl_matrix3dff_conv_3x3(pool1_2,fconv2_2,bias2_2,1,1,PADDING_VALID);

	//free filter and pool
         dl_matrix3d_free(fconv2_1);
         dl_matrix3d_free(fconv2_2);
         dl_matrix3d_free(pool1_1);
         dl_matrix3d_free(pool1_2);
	 dl_matrix3d_free(bias2_1);
         dl_matrix3d_free(bias2_2);
	//concat
	dl_matrix3d_t *out2Use = dl_matrix3d_alloc(1,8,8,48);//what? 1,8,8,48?
        out2Use=dl_matrix3d_concat(out2_1,out2_2);
        dl_matrix3d_free(out2_1);
        dl_matrix3d_free(out2_2);
//printf("\n[RNET] CONCAT  N: %i W: %i H: %i C: %i\n",out2->n,out2->w,out2->h,out2->c);
        
//printf("\n[RNET] DONE CONV 2!!\n");
        //maxpool and continue
      dl_matrix3d_t *pool2;
	pool2=dl_matrix3d_pooling(out2Use,1,1,2,2,PADDING_VALID,DL_POOLING_MAX);
      dl_matrix3d_free(out2Use);
      
      dl_matrix3d_t *prelu2 = dl_matrix3d_alloc(1,1,1,48);
//printf("\n[RENT] conv2 bias init\n");
        getrnet_prelu2_0(prelu2);
        dl_matrix3d_p_relu(pool2,prelu2);
//printf("\nPRELU 2 Done!\n");
        dl_matrix3d_free(prelu2);  

//printf("\n\n[RNET] global pool 2 done!!\n\n");
//printf("\n[RNET] SHAPE of pool2 N: %i, %i, %i, %i\n",pool2->n,pool2->w,pool2->h,pool2->c);

//========================
//	BIAS COMES FIRST
//========================

        //=====================================
	// Rnet LAYER 3 (64,2,2,48)
      
      //init
      dl_matrix3d_t *pool3;
      dl_matrix3d_t *fconv3_1 = dl_matrix3d_alloc(32,2,2,48);
      dl_matrix3d_t *fconv3_2 = dl_matrix3d_alloc(32,2,2,48);
      dl_matrix3d_t *bias3 = dl_matrix3d_alloc(1,1,1,64);

        getrnet_cut0_conv3_0(fconv3_1);
	getrnet_cut1_conv3_0(fconv3_2);
//printf("\n[RENT] conv3 init\n");        
        getrnet_conv3_1(bias3);

        //==========
        //conv split 
        //2 for in (pool shape: (1,1,1,48) ) *0.5 = (1,1,1,24) 
        //
		
	dl_matrix3d_t *pool2_1 = dl_matrix3d_alloc(1,2,2,48);
        dl_matrix3d_t *pool2_2 = dl_matrix3d_alloc(1,2,2,48);

        int px3 = (pool2->h)/2;
        int py3 = (pool2->w)/2;
        int pw3 = pool2_1->w;
        int ph3 = pool2_1->h;

        dl_matrix3d_slice_copy(pool2_1, pool2, 0, 0, pw3, ph3);
        dl_matrix3d_slice_copy(pool2_2, pool2, px3, py3, pw3, ph3);

        // 2 for bias
        dl_matrix3d_t *bias3_1 = dl_matrix3d_alloc(1,1,1,32);
        dl_matrix3d_t *bias3_2 = dl_matrix3d_alloc(1,1,1,32);

        int n3 = ((bias3->c)/2);
 	for(int i=0;i<n3;i++)
                bias3_1->item[i] = bias3->item[i];
        for(int i=n3;i<(n3*2);i++)
             bias3_2->item[i-n3] = bias3->item[i];      
        dl_matrix3d_free(bias3);

	//use 2 for filter done earlier
        dl_matrix3d_t *out3_1;
        dl_matrix3d_t *out3_2;

out3_1=dl_matrix3dff_conv_3x3(pool2_1,fconv3_1,bias3_1,1,1,PADDING_VALID);
out3_2=dl_matrix3dff_conv_3x3(pool2_2,fconv3_2,bias3_2,1,1,PADDING_VALID);

//printf("[RNET] SHAPE of out3_2 N: %i  W: %i  H:%i  C: %i",out3_2->n,out3_2->w,out3_2->h,out3_2->c);

	//make some room (pool/filter)
        dl_matrix3d_free(fconv3_1);
        dl_matrix3d_free(fconv3_2);
        dl_matrix3d_free(pool2_1);
        dl_matrix3d_free(pool2_2);
	dl_matrix3d_free(bias3_1);
        dl_matrix3d_free(bias3_2);

        //concat
        dl_matrix3d_t *out3Use = dl_matrix3d_alloc(1,2,2,64);
        out3Use=dl_matrix3d_concat(out3_1,out3_2);
  
printf("\n[RNET] SHAPE of out3Use N: %i  W: %i  H:%i  C: %i",out3Use->n,out3Use->w,out3Use->h,out3Use->c);

          dl_matrix3d_free(out3_1);
          dl_matrix3d_free(out3_2);
  //make sure its got the right shape
  	
//printf("\n[RNET] DONE CONV 3!!\n");
        pool3=dl_matrix3d_pooling(out3Use,1,1,2,2,PADDING_VALID,DL_POOLING_MAX);
        dl_matrix3d_free(out3Use);
	dl_matrix3d_t *prelu3 = dl_matrix3d_alloc(1,1,1,64);
        getrnet_prelu3_0(prelu3);
	dl_matrix3d_p_relu(pool3,prelu3);
        dl_matrix3d_free(prelu3);

//printf("\n\n[RNET] global pool 3 done!!\n\n");
 
        //=====================================
	// Rnet layer 4 --> FC 
	//layer setup? orig shape: (128,576)
	//			1/8 of that: (128,72)
	//			  1/12 (128,48) <-can alloc 20
	//		1/16 (128,36) <--divides evenly with pool3 & bias
	//		pool3 (64)/16: 4
	//		fc bias split: 128/16 =  8 

	//init var for loop
        int fcOutC = 36;
	dl_matrix3d_t *fcOut= dl_matrix3d_alloc(1,1,1,128);//576
	dl_matrix3d_t *fcBias = dl_matrix3d_alloc(1,1,1,128);
        dl_matrix3d_t *pool3Part = dl_matrix3d_alloc(1,1,1,4);//1/16 of 64
        dl_matrix3d_t *fcBiasPart= dl_matrix3d_alloc(1,1,1,8);//1/16 of 128     
        dl_matrix3d_t *fcOutPart = dl_matrix3d_alloc(1,128,36,1);
        dl_matrix3d_t *fcInStep = dl_matrix3d_alloc(1,1,128,36);

	getrnet_fc_1(fcBias);

	for(int i=0;i<16;i++)
	{
	//take 1 step
	//call the correct function
	  switch(i)
	  {
	    case 0:
	    	    getrnet_cut0_fc_0(fcInStep);
		    break;
            case 1:
                   getrnet_cut1_fc_0(fcInStep);
                    break;
            case 2:
                    getrnet_cut2_fc_0(fcInStep);
                    break;
            case 3:
                    getrnet_cut3_fc_0(fcInStep);
                    break;
            case 4:
                    getrnet_cut4_fc_0(fcInStep);
                    break;
            case 5:
                    getrnet_cut5_fc_0(fcInStep);
                    break;
            case 6:
                    getrnet_cut6_fc_0(fcInStep);
                    break;
            case 7:
                    getrnet_cut7_fc_0(fcInStep);
                    break;
            case 8:
                    getrnet_cut8_fc_0(fcInStep);
                    break;
            case 9:
                    getrnet_cut9_fc_0(fcInStep);
                    break;
            case 10:
                    getrnet_cut10_fc_0(fcInStep);
                    break;
            case 11:
                    getrnet_cut11_fc_0(fcInStep);
                    break;
            case 12:
                    getrnet_cut12_fc_0(fcInStep);
                    break;
            case 13:
                    getrnet_cut13_fc_0(fcInStep);
                    break;
            case 14:
                    getrnet_cut14_fc_0(fcInStep);
                    break;
            case 15:
                    getrnet_cut15_fc_0(fcInStep);
                    break;

	    default:
		    printf("HOW DID I GET HERE?");
		    break;
	  }

	
	int fcNb = ((fcBias->c)/16)*i;//+step;
        int fcNp = ((pool3->c)/16)*i;//+step;
        int nFb = fcNb+8;
        int nfp = fcNp+4;
        //printf("\n[RNET] FC steps fcNb:%i fcNp: %i i: %i",fcNb,fcNp,i);
	
	if(i == 0)
	  {
		//special case for 0th term.... 0 to n
		for(int n=0;n<fcNb+nFb;n++)
                	fcBiasPart->item[n]=fcBias->item[n];
        	for(int n=0;n<fcNp+nfp;n++)
		        pool3Part->item[n]=pool3->item[n];
	  }
	else
	{      // n to n + step
               for(int n=fcNb; n <  nFb; n++)
                    fcBiasPart->item[n-fcNb]=fcBias->item[n];
		for(int n=fcNp;n < nfp;n++)
                   pool3Part->item[n-fcNp]=pool3->item[n];
	}
	dl_matrix3dff_fc_with_bias(fcOutPart,pool3Part,fcInStep,fcBiasPart);
	
        int c_outN = ((fcOut->c)/16)*i;
        int n_outN = c_outN+8;//36;
	//dirty copy that bitch
	for(int k=c_outN; k < n_outN ;k++)
		fcOut->item[k] =fcOutPart->item[k-c_outN]; 
	}
dl_matrix3d_free(fcInStep);	
dl_matrix3d_free(pool3Part);
dl_matrix3d_free(fcBiasPart);
	
//for(int i=0;i<500;i++)
//	printf("\n[RNET] FC OUT --> %f",fcOut->item[i]);

	dl_matrix3d_t *fc_prelu = dl_matrix3d_alloc(1,1,1,128);
	getrnet_prelu4_0(fc_prelu);
        dl_matrix3d_p_relu(fcOut,fc_prelu);

        //===========================================
        // Rnet layer 5 --> Scores and Bounding boxes

	//Scores
	dl_matrix3d_t *score_out = dl_matrix3d_alloc(1,1,1,2);
	dl_matrix3d_t *score_filt = dl_matrix3d_alloc(128,1,1,2);
	getrnet_score_0(score_filt);
	getrnet_score_1(score_out);
	
 	dl_matrix3dff_dot_product(score_out, fcOut, score_filt);
	dl_matrix3d_free(score_filt);

	dl_matrix3d_softmax(score_out);
printf("\n\n[RNET] =====GOT SCORES!=====\n\n");
for(int i=0;i<2;i++){
printf("\n[RNET] score C: %i  i: %i val: %f",score_out->c,i,score_out->item[i]);
}
	//Bounding Boxes
	dl_matrix3d_t *bbox_filt = dl_matrix3d_alloc(128,1,1,4);
	dl_matrix3d_t *fbbox_1 = dl_matrix3d_alloc(64,1,1,4);
	dl_matrix3d_t *fbbox_2 = dl_matrix3d_alloc(64,1,1,4);

	dl_matrix3d_t *bbox_out = dl_matrix3d_alloc(1,1,1,4);

	getrnet_cut0_bbox_pred_0(fbbox_1);
        getrnet_cut1_bbox_pred_0(fbbox_2);

	bbox_filt = dl_matrix3d_concat(fbbox_1,fbbox_2);
	dl_matrix3d_free(fbbox_1);
	dl_matrix3d_free(fbbox_2);
//	getrnet_bbox_pred_1(bbox_out);

	dl_matrix3dff_dot_product(bbox_out,fcOut,bbox_filt);
	dl_matrix3d_free(fcOut);
	dl_matrix3d_free(bbox_filt);

initOut( score_out->w,score_out->h,bbox_out->w,bbox_out->h);//re-init global out
	setOutput(score_out,0);
	setOutput(bbox_out,1);
	dl_matrix3d_free(score_out);
	dl_matrix3d_free(bbox_out);	
//printf("[RNET] =======GOT BBOX=========="); 

printf("\n[RNET] BYE!\n");
xSemaphoreGive(skeletonKey);
vTaskDelay(1000/portTICK_PERIOD_MS);
//vTaskSuspend(NULL);
vTaskDelete(NULL);
 }
for(;;){}

 	
}
//================================================================================
dl_matrix3d_t *normalizeData(dl_matrix3du_t *in,int w , int h)
{
	//follow the normalization done in Caffe/Python inference demo
	dl_matrix3d_t *out = dl_matrix3d_alloc(1,w,h,in->c);
	int n=h*w*in->c;
	for (int i=0; i<n;i++)
	{
	 out->item[i]=((in->item[i]*1.0)-128.)/128.;
	}
return out;
}

box_array_t *pnet_forward(dl_matrix3du_t *image, fptp_t min_face, fptp_t pyramid, net_config_t *config)
{ /*{{{*/

	skeletonKey=xSemaphoreCreateMutex();
 
TaskHandle_t pHandle;
printf("\n\nHELLO P NET FORWARD!\n\n");
   // mtmn_net_t *out;
    fptp_t scale = 1.0f * config->w / min_face;
    image_list_t sorted_list[4] = {NULL};
    image_list_t *origin_head[4] = {NULL}; // store original point to free
    image_list_t all_box_list = {NULL};
    box_array_t *pnet_box_list = NULL;
    box_t *pnet_box = NULL;
    //bool found = false;
    //printf("\n\nFWD INIT!\n\n");

    int width = round(image->w *scale);// .04);
    int height =round(image->h *scale);// .07);
    dl_matrix3d_t *normal;

    dl_matrix3du_t *in = dl_matrix3du_alloc(1, width, height, image->c);
    for (int i = 0; i < 4; i++)
    {
        if (DL_IMAGE_MIN(width, height) <= config->w )
	{
//            	printf("\n\n pnet fwd BREAK!!\n\n");
		break;
	}
image_resize_linear(in->item, image->item, width, height, in->c, image->w, image->h);
        in->h = height;
        in->w = width;
        in->stride = in->w * in->c;
//printf("\n[PNET FWD] SETUPIMAGE %i X %i  \n ",width,height);
//===================================
// NORMALIZE DATA
//=================================== 
	 normal = normalizeData(in,in->w,in->h);
	//free(in);
esp_err_t startTimer=  esp_timer_init();

xTaskCreatePinnedToCore(&pnetCore1,"core2Print",45000,normal,5,&pHandle,1);//normal,5,&pHandle,1);   24000
uint64_t pnetTime = esp_timer_get_time();
  
//vTaskDelay(1000/portTICK_PERIOD_MS);
//out =xTaskCreate(&pnetCore1,"core2pnet",50000,in,1,NULL); 
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
//vTaskDelay(1000/portTICK_PERIOD_MS);



printf("\n[PNET FWD] PNET CONV TIME:: %lli \n",pnetTime);
esp_err_t deleteTime=esp_timer_deinit();


int canGo=0;
if(output)
	canGo = (output->category->n > 0) ? 1:0;
//printf("[PNET FWD] canGo %i  ",canGo);

	if(canGo ==1)//if(output)//!=NULL )
	{
	//vTaskDelete(&pHandle);
//printf("\n\nSKELETON KEY: %p\n\n",skeletonKey);
//printf("\n\n[PNET FWD]..getting valid boxes..\n\n\n");
//printf("\n[PNET FWD] OUTPUT SHAPE \nscore--> %i,%i,%i,%i \nbbox--> %i,%i,%i,%i \n",output->category->n,output->category->w,output->category->h,output->category->c,output->offset->n,output->offset->w,output->offset->h,output->offset->c);

/*
for(int i=0;i<2;i++)
        printf("\nscore: i %i --> %f",i,output->category->item[i]);
for(int i=0;i<4;i++)
        printf("\nbbox: i %i --> %f",i,output->offset->item[i]);
*/
//make sure the output is valid
for(int i=0;i<2;i++)
        if(isnan(output->category->item[i]))
		continue;
for(int i=0;i<4;i++)
	if(isnan(output->offset->item[i]))
		continue;
printf("\n\n[PNET FWD]..getting valid boxes..\n\n\n");

/*
if(isnan(output->category->item[0]))
{
        printf("\n[PNET FWD] BAD SCORE.....skipping...\n");
        continue;
}
*/
vTaskDelay(1000/portTICK_PERIOD_MS);
            origin_head[i] = image_get_valid_boxes(output->category->item, 
			    output->offset->item ,
			    NULL, 
			    output->category->w,  
			    output->category->h, 
			    1,
			    &config->w,
			    config->threshold.score,
                            2,
			    2,
                            scale,
                                                   //scale,
                                                   false);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	//xSemaphoreGive(skeletonKey);
//	printf("\n\n[PNET FWD] GAVE KEY\n\n");
	    if (origin_head[i])
            {
//printf("\n[PNET FWD] head: %i\n",origin_head[i]->len);
printf("\n[PNET FWD] NMS STARTING!\n");
                image_sort_insert_by_score(&sorted_list[i], origin_head[i]);

                image_nms_process(&sorted_list[i], 0.0001, true);
//printf("\n[PNET FWD] NMS COMPLETE!\n");
            
	    }
	  
	    // dl_matrix3d_free(output)--> DONT DO THIS ITS A GLOBAL
   	setOutput(NULL,3);
        scale *= pyramid;
        width = round(image->w * scale);
        height = round(image->h * scale);
     	}
    

    for (int i = 0; i < 4; i++)
        image_sort_insert_by_score(&all_box_list, &sorted_list[i]);
//printf("\n[PNET FWD] SORTED NMS\n\n");
    image_nms_process(&all_box_list, config->threshold.nms, false);
//printf("\n[PNET FWD] nms list len: %i",all_box_list.len);
    if (all_box_list.len)
    {
        if (all_box_list.len > config->threshold.candidate_number)
            all_box_list.len = config->threshold.candidate_number;
        image_calibrate_by_offset(&all_box_list, image->h, image->w);
        pnet_box_list = (box_array_t *)dl_lib_calloc(1, sizeof(box_array_t), 0);
        pnet_box = (box_t *)dl_lib_calloc(all_box_list.len, sizeof(box_t), 0);
        image_box_t *t = all_box_list.head;
        // no need to store landmark
        for (int i = 0; i < all_box_list.len; i++, t = t->next)
	{
	  pnet_box[i] = t->box;
        pnet_box_list->box = pnet_box;
        pnet_box_list->len = all_box_list.len;
	printf("[PNET FWD] LIST LEN: %i",all_box_list.len);
	/*
  	 	if(pnet_box_list->len <= 0)
		{
		// printf("\n[PNET FWD] nothing found, lets retry..\n\n");
		}
   		else
       		{
		printf("\n[PNET FWD] nothing found, lets retry..\n\n");
       		}
	*/ 
	}	
      for (int i = 0; i < 4; i++)
      {
        if (origin_head[i])
        {
            	dl_lib_free(origin_head[i]->origin_head);
            	dl_lib_free(origin_head[i]);
        }
      }
    }

//xSemaphoreGive(skeletonKey);
//free(in);
    return pnet_box_list;
} /*}}}*/



box_array_t *rnet_forward(dl_matrix3du_t *image, box_array_t *net_boxes, net_config_t *config)
{ /*{{{*/

    skeletonKey=xSemaphoreCreateMutex();

    int valid_count = 0;
    image_list_t valid_list = {NULL};
    image_list_t sorted_list = {NULL};
    dl_matrix3du_t *resized_image;
    dl_matrix3du_t *sliced_image;
    image_box_t *valid_box = NULL;
    box_t *net_box = NULL;
    box_array_t *net_box_list = NULL;
    TaskHandle_t rHandle;
    dl_matrix3d_t *normal;
    if (NULL == net_boxes)
        return NULL;//redundant

    valid_box = (image_box_t *)dl_lib_calloc(config->threshold.candidate_number, sizeof(image_box_t), 0);
    resized_image = dl_matrix3du_alloc(1, config->w, config->h, image->c);

    image_rect2sqr(net_boxes, image->w, image->h);
    printf("\n[RNET FWD] rect2sqr done, GOING TO FOR LOOP\n");
	printf("conf W:%i image C: %i  box len: %i ",config->w,image->w,net_boxes->len);
     printf("\n\n  X: %f  Y: %f  W: %f  H: %f \n\n",round(net_boxes->box[0].box_p[0]),round(net_boxes->box[0].box_p[1]), round(net_boxes->box[0].box_p[2]),round(net_boxes->box[0].box_p[3]) );


    for (int i = 0; i < net_boxes->len; i++)
    {
        int x = round(net_boxes->box[i].box_p[0]);
        int y = round(net_boxes->box[i].box_p[1]);
        int w =abs( round(net_boxes->box[i].box_p[2]) - x + 1);
        int h = abs(round(net_boxes->box[i].box_p[3]) - y + 1);
        sliced_image = dl_matrix3du_alloc(1, w, h, image->c);
	printf("\n[RNET FWD] X: %i  Y: %i  W: %i  H: %i",x,y,w,h); 

bool chckMem1 =heap_caps_check_integrity_all(true);
        dl_matrix3du_slice_copy(sliced_image, image, x, y, w, h);

image_resize_linear(resized_image->item, sliced_image->item, config->w, config->h, image->c, w, h);
//printf("\n[RNET FWD] RESIZED..\n");
	//NORMALIZE IMAGE..
	normal = normalizeData(resized_image,resized_image->w,resized_image->h);
//printf("\n\n[RNET FWD] normalized c: %i",normal->c);

esp_err_t startTimer=  esp_timer_init();

xTaskCreatePinnedToCore(&rnetCore1,"core2rNet",30000,normal,5,&rHandle,1);//29500
uint64_t rnetTime = esp_timer_get_time();
	    vTaskDelay(1000/portTICK_PERIOD_MS);
	    vTaskDelay(1000/portTICK_PERIOD_MS);

printf("\n[RNET FWD] RNET CONV TIME:: %lli \n",rnetTime);
esp_err_t deleteTime=esp_timer_deinit();
            
	    vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            vTaskDelay(1000/portTICK_PERIOD_MS);

//mtmn_net_t *out = rnet_lite_f_with_score_verify(resized_image, config->thr)

        if (output)
        {
//printf("\n[RNET FWD] ANALYZE RNET OUT\n\n");
            assert(output->category->stride == 2);
            assert(output->offset->stride == 4);
            assert(output->offset->c == 4);
            valid_box[valid_count].score = output->category->item[1];
            valid_box[valid_count].box = net_boxes->box[i];
            valid_box[valid_count].offset.box_p[0] = output->offset->item[0];
            valid_box[valid_count].offset.box_p[1] = output->offset->item[1];
            valid_box[valid_count].offset.box_p[2] = output->offset->item[2];
	    valid_box[valid_count].offset.box_p[3] = output->offset->item[3];
            valid_box[valid_count].next = &(valid_box[valid_count + 1]);
            valid_count++;
            //dl_matrix3d_free(out->category);
            //dl_matrix3d_free(out->offset);
            //dl_lib_free(out);
        }
        dl_matrix3du_free(sliced_image);

        if (valid_count > config->threshold.candidate_number - 1)
            break;
    }

    dl_matrix3du_free(resized_image);
    if (valid_count)
        valid_box[valid_count - 1].next = NULL;
    else
        valid_box[0].next = NULL;
    valid_list.head = valid_box;
    valid_list.len = valid_count;
    image_sort_insert_by_score(&sorted_list, &valid_list);
    image_nms_process(&sorted_list, config->threshold.nms, false);
    if (sorted_list.len)
    {
	printf("[RNET FWD] VALID OUTPUT FROM RNET!\n\n");
        image_calibrate_by_offset(&sorted_list, image->h, image->w);

        net_box_list = (box_array_t *)dl_lib_calloc(1, sizeof(box_array_t), 0);
        net_box = (box_t *)dl_lib_calloc(sorted_list.len, sizeof(box_t), 0);

        image_box_t *t = sorted_list.head;

        for (int i = 0; i < sorted_list.len; i++, t = t->next)
        {
            net_box[i] = t->box;
        }
        net_box_list->box = net_box;
        net_box_list->len = sorted_list.len;
    }

    dl_lib_free(valid_box);
    return net_box_list;
} /*}}} */



static void draw_face_boxes(dl_matrix3du_t *image_matrix, box_array_t *boxes, int face_id){
    int x, y, w, h, i;
    uint32_t color = FACE_COLOR_YELLOW;
    if(face_id < 0){
        color = FACE_COLOR_RED;
    } else if(face_id > 0){
        color = FACE_COLOR_GREEN;
    }
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    for (i = 0; i < boxes->len; i++){
        // rectangle box
        x = (int)boxes->box[i].box_p[0];
        y = (int)boxes->box[i].box_p[1];
        w = (int)boxes->box[i].box_p[2] - x + 1;
        h = (int)boxes->box[i].box_p[3] - y + 1;
        fb_gfx_drawFastHLine(&fb, x, y, w, color);
        fb_gfx_drawFastHLine(&fb, x, y+h-1, w, color);
        fb_gfx_drawFastVLine(&fb, x, y, h, color);
        fb_gfx_drawFastVLine(&fb, x+w-1, y, h, color);
#if 0
        // landmark
        int x0, y0, j;
        for (j = 0; j < 10; j+=2) {
            x0 = (int)boxes->landmark[i].landmark_p[j];
            y0 = (int)boxes->landmark[i].landmark_p[j+1];
            fb_gfx_fillRect(&fb, x0, y0, 3, 3, color);
        }
#endif
    }
}

void initConf(int id)
{
//Pnet first
if(id==1)
{
	currentConfig->w=12;
	currentConfig->h=12;
	currentConfig->threshold.score=0.07;//from python tests in caffe
	currentConfig->threshold.nms=0.07;
	currentConfig->threshold.candidate_number=100;//10, 20.. inf from caffe;
	   }
//Rnet 2nd..
else if(id==2)
{
    currentConfig->w = 24;
    currentConfig->h = 24;
    currentConfig->threshold.score = 0.369;//from python tests in caffe
    currentConfig->threshold.nms=0.369;//
    currentConfig->threshold.candidate_number=10;//inf from caffe
}
else if(id == 3)
{
    currentConfig->w = 48;
    currentConfig->h = 48;
    currentConfig->threshold.score = 0.0;//0.7 --> from ESP, .96 from caffe
    currentConfig->threshold.nms=0.0;//0.7 --> FROM ESP
    currentConfig->threshold.candidate_number=1;//CHANGE LATER, add more
}
else 
{
	//should not see this
	printf("\n\n\nWRONG ID MAN\n\n\n");
}
//return no need, global config;
}

//dl_matrix3du_t *takePic()
void analyzeImage()
{
  camera_fb_t * pic = NULL;  
  esp_err_t err = ESP_OK;
  // Take Picture with Camera
    jpg_chunking_t jchunk = {0};
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    
vTaskDelay(1000/portMAX_DELAY);
  pic = esp_camera_fb_get();  
    if(!pic)
    {
      printf("ERR TAKING PICTURE");
       return;//NULL;
    }
    

    if(pic->format == PIXFORMAT_JPEG)
    {
       // jpg_chunking_t jchunk = {0};
        err = frame2jpg_cb(pic, 80/*quality*/, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
      	
	//saveImage(pic,0);
	
	if(err == ESP_FAIL)
        {
          printf("ERROR IN JPEG: ");
          //ESP_LOGE(TAG, "JPEG compression failed");
          esp_camera_fb_return(pic);
          //res = ESP_FAIL;
	  return;//NULL;
	}
	else
	{
	
	      _jpg_buf_len = pic->len;
                _jpg_buf = pic->buf;
	}
    }
   //allocate array

   dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, pic->width, pic->height, 3);
   if(!image_matrix)
   {
     dl_matrix3du_free(image_matrix);
     esp_camera_fb_return(pic);
     printf("image matrix allocation failed");
     return;//NULL;
   }

   //get image data into the array
  //fmt2jpg
//fmt2jpg(src,src_len,width,height,format,qualityy, uint8_t ** out, size_t * out_len);
//   if(!fmt2jpg(pic->buf,pic->len,pic->w ,pic->h,pic->format,10,image_matrix->item
  //fmt2rgb888
//  if(!fmt2jpg(pic->buf,pic->width*pic->height*3, pic->width,pic->height,PIXFORMAT_RGB888,10,&_jpg_buf,&jpg_buf_len))

   
  
   //put image data into dataframe
  if(!fmt2rgb888(pic->buf,pic->len,pic->format,image_matrix->item))
//fmt2jpg(pic->buf,pic->len,pic->width,pic->height,pic->pic->format,10,image_matrix->item))
  {
    dl_matrix3du_free(image_matrix);
    esp_camera_fb_return(pic);
    printf("FORMAT 2 RGB 888 FAILED");
    return;// NULL;
  }
	
printf("\n\nhi!\n");
initConf(1);
initOut(1,1,1,1);
//printf("\n\nINIT P conf\n\n");


//some configurations
float minLeaf=80; //120;//80;
float pyramid=.707;//.5;//.707;

printf("\n[GET IMG] IMG SHAPE %i %i %i %i",image_matrix->n,image_matrix->w,image_matrix->h,image_matrix->c);


//pnet forward -->
box_array_t *pBoxes;
//esp_err_t startTimer=  esp_timer_init();
pBoxes=pnet_forward( image_matrix,minLeaf,pyramid,currentConfig );
//uint64_t pnetTime = esp_timer_get_time();

uint32_t free1 = esp_get_free_heap_size();
printf("\n[GET IMG] After Pnet fwd: %i \n",free1);

if(pBoxes != NULL )//|| pBoxes->box[0].box_p[0]< 0  )
{

//printf("\n[GET IMG] P box X: %f \n",pBoxes->box[0].box_p[0]);
//printf("\n[GET IMG] P box Y: %f \n",pBoxes->box[0].box_p[1]);
//printf("\n[GET IMG] P box W: %f \n",pBoxes->box[0].box_p[2]);
//printf("\n[GET IMG] P box H: %f \n",pBoxes->box[0].box_p[3]);
	if( pBoxes->box[0].box_p[0]> 0 && pBoxes->box[0].box_p[3] > 0 )
	{
		setOutput(NULL,3);
	        initConf(2);//init Rnet config
		box_array_t *rBoxes;
		rBoxes=rnet_forward(image_matrix, pBoxes,currentConfig);
//	printf("\n[GET IMG] Out from r-net!\n");
	       if(rBoxes!=NULL)
	       {
//printf("\n[GET IMG] R box X: %f \n",rBoxes->box[0].box_p[0]);
//printf("\n[GET IMG] R box Y: %f \n",rBoxes->box[0].box_p[1]);
//printf("\n[GET IMG] R box W: %f \n",rBoxes->box[0].box_p[2]);
//printf("\n[GET IMG] R box H: %f \n",rBoxes->box[0].box_p[3]);
      if(rBoxes->box[0].box_p[0]< 50000000 && rBoxes->box[0].box_p[1]< 50000000 )
      {
	if( rBoxes->box[0].box_p[0] > -1 )
	          {
			  printf("\n[GET IMG] VALID RNET BOX!!!!!\n ");


			  draw_face_boxes(image_matrix,rBoxes,0);      

	       
printf("\n\n\n[GET IMG] LEAF FOUND @:\n");	
for (int i=0;i<rBoxes->len;i++){
printf("\n[GET IMG] R box X: %f \n",rBoxes->box[i].box_p[0]);
printf("\n[GET IMG] R box Y: %f \n",rBoxes->box[i].box_p[1]);
printf("\n[GET IMG] R box W: %f \n",rBoxes->box[i].box_p[2]);
printf("\n[GET IMG] R box H: %f \n",rBoxes->box[i].box_p[3]);
printf("\n\n\n");
}
	  //===========================
          //get back to jpeg
	  //

//if(!fmt2jpg_cb(image_matrix->item, pic->width*pic->height*3, pic->width, pic->height, PIXFORMAT_RGB888, 80,jpg_encode_stream, &jchunk)){
		// &_jpg_buf, &_jpg_buf_len)){
	//	draw_face_boxes(image_matrix,pBoxes,0);
		//saveImage(pic,0);

		//printf("BACK 2 JPG FAIL.");
		//RNET
		saveImage(pic,1);
         //=============================	
//		}
// 	else
//    	{ 
   	  
//err = frame2jpg_cb(pic, 80/*quality*/, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
  //      _jpg_buf_len = pic->len;
 //     _jpg_buf = pic->buf;
 	  //camera_fb_t * pic2 = NULL;
	//  pic2 =  esp_camera_fb_get();
	printf("\n[GET IMG] PIC FORMAT: %i \n" ,pic->format);
	
	//saveImage(pic,2);
//	}

     }//check rBox > 0 
    }//check rBox
   }//if rbox NOT NULL
  }//pBox check
}//pBox NOT NULL

 	   //free the camera frame for next picture
  	  esp_camera_fb_return(pic);
	  pic=NULL;
	  dl_matrix3du_free(image_matrix);
//return image_matrix; 
}


void app_main()
{
bool setup = false;
while(true)
{
//    printf("Hello world!\n");
	if(!setup)
	{
		bool init= cameraConfig();
		if(init)
		{
			printf("CAM INIT");

		}
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

      	 skeletonKey = xSemaphoreCreateMutex();
	  setup = true;
	}

	analyzeImage();
/* //get the image into main!
	dl_matrix3du_t *image = takePic();	
	if(image->w == 0)
	{
		printf("image capture fail..\n");
		printf("Restarting...");
		esp_restart();	
	}
*/
vTaskDelay(1000/portTICK_PERIOD_MS);
printf("\n[MAIN] did it work??\n");
    //dl_matrix3du_free(image);
    fflush(stdout);
}//end forever while loop

}//endmain
/*mutex = xSemaphoreCreateMutex(); */

/*
box_array_t *onet_forward(dl_matrix3du_t *image, box_array_t *net_boxes, net_config_t *config)
{ / *{{{ * /

    skeletonKey=xSemaphoreCreateMutex();
    int valid_count = 0;
    image_list_t valid_list = {NULL};
    image_list_t sorted_list = {NULL};
    dl_matrix3du_t *resized_image;
    dl_matrix3du_t *sliced_image;
    image_box_t *valid_box = NULL;
    box_t *net_box = NULL;
    fptp_t *net_score = NULL;
    landmark_t *net_landmark = NULL;
    box_array_t *net_box_list = NULL;
    dl_matrix3d_t *normal;
    TaskHandle_t oHandle;

    if (NULL == net_boxes)
        return NULL;//redundant

    valid_box = (image_box_t *)dl_lib_calloc(config->threshold.candidate_number, sizeof(image_box_t), 0);
    resized_image = dl_matrix3du_alloc(1, config->w, config->h, image->c);

    image_rect2sqr(net_boxes, image->w, image->h);
    for (int i = 0; i < net_boxes->len; i++)
    {
        int x = round(net_boxes->box[i].box_p[0]);
        int y = round(net_boxes->box[i].box_p[1]);
        int w = round(net_boxes->box[i].box_p[2]) - x + 1;
        int h = round(net_boxes->box[i].box_p[3]) - y + 1;
        sliced_image = dl_matrix3du_alloc(1, w, h, image->c);

        dl_matrix3du_slice_copy(sliced_image, image, x, y, w, h);

        image_resize_linear(resized_image->item, sliced_image->item, config->w, config->h, image->c, w, h);
printf("\n[ONET  FWD] RESIZED\n");
	//=========
	//normalize
	//=========
   normal = normalizeData(resized_image,resized_image->w,resized_image->h);
printf("\n[ONET FWD] NORMALIZED..\n");
xTaskCreatePinnedToCore(&onetCore1,"core2Onet",65000,normal,5,&oHandle,1);//65000
vTaskDelay(1000/portTICK_PERIOD_MS);
/ *
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelay(1000/portTICK_PERIOD_MS);
* /
        if (output!=NULL)
        {
	    printf("[ONET FWD] START POST PROCESS!");
            assert(output->category->stride == 2);
            assert(output->offset->stride == 4);
            assert(output->offset->c == 4);
            valid_box[valid_count].score = output->category->item[1];
            valid_box[valid_count].box = net_boxes->box[i];
            valid_box[valid_count].offset.box_p[0] = output->offset->item[0];
            valid_box[valid_count].offset.box_p[1] = output->offset->item[1];
            valid_box[valid_count].offset.box_p[2] = output->offset->item[2];
            valid_box[valid_count].offset.box_p[3] = output->offset->item[3];
            //assert(output->landmark->stride == 10);//wtf?
       //memcpy(&(valid_box[valid_count].landmark), output->landmark->item, sizeof(landmark_t));

          valid_box[valid_count].next = &(valid_box[valid_count + 1]);
            valid_count++;
	/ *
            dl_matrix3d_free(out->category);
            dl_matrix3d_free(out->offset);
            dl_matrix3d_free(out->landmark);
            dl_lib_free(out);
        * /
	}
        //dl_matrix3du_free(sliced_image);

        if (valid_count > config->threshold.candidate_number - 1)
            break;
    }

    dl_matrix3du_free(resized_image);

    if (valid_count)
        valid_box[valid_count - 1].next = NULL;
    else
        valid_box[0].next = NULL;

    valid_list.head = valid_box;
    valid_list.len = valid_count;
    image_sort_insert_by_score(&sorted_list, &valid_list);

    if (sorted_list.len)
    {
        image_landmark_calibrate(&sorted_list);

        image_calibrate_by_offset(&sorted_list, image->h, image->w);

        image_nms_process(&sorted_list, config->threshold.nms, false);

        net_box_list = (box_array_t *)dl_lib_calloc(1, sizeof(box_array_t), 0);
        net_box = (box_t *)dl_lib_calloc(sorted_list.len, sizeof(box_t), 0);
        net_score = (fptp_t *)dl_lib_calloc(sorted_list.len, sizeof(fptp_t), 0);
//net_landmark = (landmark_t *)dl_lib_calloc(sorted_list.len, sizeof(landmark_t), 0);

        image_box_t *t = sorted_list.head;

        for (int i = 0; i < sorted_list.len; i++, t = t->next)
        {
            net_box[i] = t->box;
            net_score[i] = t->score;
            //net_landmark[i] = t->landmark;
        }

        net_box_list->score = net_score;
        net_box_list->box = net_box;
        //net_box_list->landmark = net_landmark;
        net_box_list->len = sorted_list.len;
    }
    dl_lib_free(valid_box);

    return net_box_list;
} / *}}}*/
/*
void onetCore1(void  *pvParam)//mtmn_net_t *in)
{
 for( ; ; )
 {
	printf("\n[ONET] WELCOME TO O-NET\n");
        dl_matrix3d_t *in =(dl_matrix3d_t*) pvParam;
        xSemaphoreTake(skeletonKey,portMAX_DELAY);
	//input -> shape (48,48, RGB) 
	// Onet LAYER 1
	//=================================
        dl_matrix3d_t *filt_c1 = dl_matrix3d_alloc(16,3,3,3);
        dl_matrix3d_t *bias1 = dl_matrix3d_alloc(1,1,1,16);
        dl_matrix3d_t *pool1;
	dl_matrix3d_t *out1;
        dl_matrix3d_t *prelu1 = dl_matrix3d_alloc(1,1,1,16);

        getonet_conv1_0(filt_c1);
        getonet_conv1_1(bias1);
        getonet_prelu1_0(prelu1);
        
	out1 = dl_matrix3dff_conv_3x3(in,filt_c1,bias1,1,1,PADDING_SAME);
        dl_matrix3d_free(in);
        
	pool1=swimming_pool(out1,2);
        dl_matrix3d_free(out1);

        dl_matrix3d_p_relu(pool1,prelu1);
        dl_matrix3d_free(prelu1);

        //=====================================
	// Onet LAYER 2 
        dl_matrix3d_t *filt_c2 = dl_matrix3d_alloc(32,3,3,16);
        dl_matrix3d_t *bias2 = dl_matrix3d_alloc(1,1,1,32);
        dl_matrix3d_t *out2;//= dl_matrix3d_alloc(1,1,1,32);
	dl_matrix3d_t *pool2;
        dl_matrix3d_t *prelu2 = dl_matrix3d_alloc(1,1,1,32);

        getonet_conv2_0(filt_c2);
        getonet_conv2_1(bias2);
        getonet_prelu2_0(prelu2);

        out2 = dl_matrix3dff_conv_3x3(pool1,filt_c2,bias2,1,1,PADDING_SAME);
        dl_matrix3d_free(pool1);

        pool2=swimming_pool(out2,2);
        dl_matrix3d_free(out2);
        
	dl_matrix3d_p_relu(pool2,prelu2);
        dl_matrix3d_free(prelu2);
bool checkMem =heap_caps_check_integrity_all(true);
        //=====================================
	// Onet LAYER 3
        dl_matrix3d_t *filt_c3 = dl_matrix3d_alloc(48,3,3,32);
        dl_matrix3d_t *bias3 = dl_matrix3d_alloc(1,1,1,48);
        dl_matrix3d_t *out3; // (1,1,1,48);
	dl_matrix3d_t *pool3;// (1,1,1,48);
        dl_matrix3d_t *prelu3 = dl_matrix3d_alloc(1,1,1,48);

        getonet_conv2_0(filt_c3);
        getonet_conv2_1(bias3);
        getonet_prelu3_0(prelu3);

        out3 = dl_matrix3dff_conv_3x3(pool2,filt_c3,bias3,1,1,PADDING_SAME);
        dl_matrix3d_free(pool2);
        
	pool3=swimming_pool(out3,2);
	dl_matrix3d_free(out3);

	dl_matrix3d_p_relu(pool3,prelu3);
        dl_matrix3d_free(prelu3);
bool checkMem1 =heap_caps_check_integrity_all(true);
//printf("\n\n[ONET] global pool 3 done!!\n\n");

        //=====================================
	// Onet layer 4 --> FC 
	/ *
	 * (64,768)
	 * 	1/8th (64.96)
	  * /
//dl_matrix3d_t *fc_bias= dl_matrix3d_alloc(1,1,1,64);//576);

	dl_matrix3d_t *fc1 = dl_matrix3d_alloc(1,64,192,1);
        dl_matrix3d_t *fc2 = dl_matrix3d_alloc(1,64,192,1);		    
	dl_matrix3d_t *fc3 = dl_matrix3d_alloc(1,64,192,1);
        dl_matrix3d_t *fc4 = dl_matrix3d_alloc(1,64,192,1);

        getonet_cut0_fc_0(fc1);
        getonet_cut1_fc_0(fc2);
	getonet_cut2_fc_0(fc3);
	getonet_cut3_fc_0(fc4);
bool hh= heap_caps_check_integrity_all(true);
	dl_matrix3d_t *fc_in1;//=dl_matrix3d_alloc(1,64,768,1);
	dl_matrix3d_t *fc_in2;

	fc_in1 = dl_matrix3d_concat(fc1,fc2);//,fc3,fc4);

        dl_matrix3d_free(fc1);
        dl_matrix3d_free(fc2);
/ *
	fc_in->n=1;
	fc_in->c=1;
	fc_in->h=768;
* /
bool tt =heap_caps_check_integrity_all(true);
/ *	fc_in1->n=1;
	fc_in1->c=1;
	fc_in1->h=384;
* /
bool yy=heap_caps_check_integrity_all(true);        
	int pooledSplitC = (pool3->c)/2;
	int pooledSplit = (pool3->stride)/2;

	dl_matrix3d_t *pool3_1 = dl_matrix3d_alloc(1,1,1,pooledSplitC);
bool fuck =heap_caps_check_integrity_all(true);	
	for(int i=0;i<pooledSplit;i++)
	{
		pool3_1->item[i]=pool3->item[i];
		//pool3->item[i]=pool3_1->item[i];
		printf("\n i: %i in: %f",i,pool3_1->item[i]); 
	}
	dl_matrix3d_t *fc_out1= dl_matrix3d_alloc(1,1,1,32);
	//HALF FC!
        
	dl_matrix3dff_fc(fc_out1,pool3_1,fc_in1);
bool fuckme =heap_caps_check_integrity_all(true);

	dl_matrix3d_free(pool3_1);
        dl_matrix3d_free(fc_in1);
printf("[ONET]\n HALF FC DONE\n");
	dl_matrix3d_t *pool3_2 = dl_matrix3d_alloc(1,1,1,pooledSplitC);
	for(int i=pooledSplit; i < pooledSplit*2; i++)
	{
         pool3_2->item[i]=pool3->item[i];
	}

	dl_matrix3d_free(pool3);
	fc_in2 = dl_matrix3d_concat(fc3,fc4);
        dl_matrix3d_free(fc3);
        dl_matrix3d_free(fc4);
        
	fc_in2->n=1;
        fc_in2->c=1;
        fc_in2->h=384;

	dl_matrix3d_t *fc_out2 = dl_matrix3d_alloc(1,1,1,32);
	//HALF FC #2
	dl_matrix3dff_fc(fc_out2,pool3_2, fc_in2);
//printf("\n[ONET] FC IN 2  -> N: %i  C: %i W: %i H: %i",fc_in2->n,fc_in2->c,fc_in2->w,fc_in2->h);
      //  dl_matrix3d_free(pool3_2);
       
       	dl_matrix3d_free(fc_in2);

	dl_matrix3d_t *fc_out;
	fc_out = dl_matrix3d_concat(fc_out1,fc_out2);

printf("\n[ONET] FC OUT -> N: %i  C: %i W: %i H: %i",fc_out->n,fc_out->c,fc_out->w,fc_out->h);
	//pool3 (1,1,1,48)
	//pool3_1 (1,1,1,24)
/ *
  esp_set_watchpoint(
  HEAP_TRACE_ALL
 * /	

//split last layer completely? 
// 1/2 full connected
// 1/2 bias
// 1/2 prelu
// 1/2 
//printf("\n\n[ONET] FC FULLY INIT!!!!!!!\n\n");

	//dl_matrix3d_t *fc_bias=dl_matrix3d_alloc(1,1,1,64);

	//dl_matrix3d_t *fc_bias1=dl_matrix3d_alloc(1,1,1,32);
	//dl_matrix3d_t *fc_bias2=dl_matrix3d_alloc(1,1,1,32);


	//dl_matrix3d_t *fc_out=dl_matrix3d_alloc(1,1,1,64);	
//	getonet_fc_1(fc_bias);       
//	dl_matrix3dff_fc_with_bias(fc_out,pool3,fc_in,fc_bias);

	//dl_matrix3dff_fc_with_bias(fc_out,pool3,fc_in,fc_bias);

//bool checkMem =heap_caps_check_integrity_all(true);

//bool checkMem1 =heap_caps_check_integrity_all(true);

printf("\n[ONET] FC DONE!!\n");

//printf("\n[ONET] FC IN -> N: %i  C: %i W: %i H: %i",fc_in->n,fc_in->c,fc_in->w,fc_in->h);

/ *
	dl_matrix3d_free(fc_bias);
	
	dl_matrix3d_free(fc_in);
	
	dl_matrix3d_free(pool3);

	dl_matrix3d_t *fc_prelu = dl_matrix3d_alloc(1,1,1,64);
	
	getonet_prelu5_0(fc_prelu);

        dl_matrix3d_p_relu(fc_out,fc_prelu);
	dl_matrix3d_free(fc_prelu);
printf("\n[ONET] FC OUT -> N: %i  C: %i W: %i H: %i",fc_out->n,fc_out->c,fc_out->w,fc_out->h);

        //===========================================
        // Onet layer 5 --> Scores and Bounding boxes

        //Scores
printf("\n[ONET] ALLOC SCORE\n\n");
//vTaskDelay(1000/portTICK_PERIOD_MS);

        dl_matrix3d_t *score_out = dl_matrix3d_alloc(1,1,1,2);
        dl_matrix3d_t *score_filt = dl_matrix3d_alloc(64,1,1,2);
       
       	getonet_score_0(score_filt);
        getonet_score_1(score_out);

        dl_matrix3dff_dot_product(score_out, fc_out, score_filt);
          
        dl_matrix3d_softmax(score_out);
	dl_matrix3d_free(score_filt);
printf("\n\n[ONET] =====GOT SCORES!=====\n\n");

//bool checkMem =heap_caps_check_integrity_all(true);


printf("\n\n[ONET] allocating bbox \n\n");
//vTaskDelay(1000/portTICK_PERIOD_MS);

        //Bounding Boxes 
	dl_matrix3d_t *bbox_out = dl_matrix3d_alloc(1,1,1,4);
	dl_matrix3d_t *bbox_fil = dl_matrix3d_alloc(64,1,1,4);//(64,1,1,4)

printf("\n[ONET] ALLOCATED bbox\n");
	getonet_bbox_pred_0(bbox_fil);
	getonet_bbox_pred_1(bbox_out);
        dl_matrix3dff_dot_product(bbox_out,fc_out,bbox_fil);

	dl_matrix3d_free(fc_out);
	dl_matrix3d_free(bbox_fil);

	initOut();
        setOutput(score_out,0);//into the global!
  	setOutput(bbox_out,1);

        dl_matrix3d_free(bbox_out);
	dl_matrix3d_free(score_out);

	dl_matrix3d_free(fc_out);
//	dl_matrix3d_free(score_out);
//	setOutput(NULL,3);
//bool checkMem =heap_caps_check_integrity_all(true);
vTaskDelay(1000/portTICK_PERIOD_MS);
* /
printf("[ONET] =======GOT BBOX=========="); 	
	
xSemaphoreGive(skeletonKey);
//vTaskDelay(1000/portTICK_PERIOD_MS);
vTaskDelete(NULL);
//uint32_t free = uxTaskGetStackHighWaterMark(NULL);//FREE STACK MEM
//uint32_t free = esp_get_free_heap_size();


 }
}
 
*/
/*
//printf("\n[ONET] FC1 INIT!\n");
        dl_matrix3d_t *fc1 = dl_matrix3d_alloc(1,64,96,1);
        dl_matrix3d_t *fc2 = dl_matrix3d_alloc(1,64,96,1);
        getonet_cut0_fc_0(fc1);
        getonet_cut1_fc_0(fc2);
//printf("\n\n[ONET] GOT FC 1 and 2 \n");
//uint32_t free = esp_get_free_heap_size();
//printf("\n[ONET] FREE MEM b4 FC: %i \n",free);
//bool checkMem =heap_caps_check_integrity_all(true);
        dl_matrix3d_t *fcFourth = dl_matrix3d_alloc(1,64,192,1);
        fcFourth=dl_matrix3d_concat(fc1,fc2);
        dl_matrix3d_free(fc1);
        dl_matrix3d_free(fc2);
        dl_matrix3d_t *fc3 = dl_matrix3d_alloc(1,64,96,1);
        dl_matrix3d_t *fc4 = dl_matrix3d_alloc(1,64,96,1);
	getonet_cut2_fc_0(fc3);
	getonet_cut3_fc_0(fc4);
	dl_matrix3d_t *fc_2ndFourth=dl_matrix3d_alloc(1,64,192,1);
//not here bool checkMem =heap_caps_check_integrity_all(true);
	fc_2ndFourth=dl_matrix3d_concat(fc3,fc4);
        dl_matrix3d_free(fc3);
        dl_matrix3d_free(fc4);
	dl_matrix3d_t *fc_half=dl_matrix3d_alloc(1,64,384,1);
	fc_half = dl_matrix3d_concat(fcFourth,fc_2ndFourth);
	dl_matrix3d_free(fcFourth);
	dl_matrix3d_free(fc_2ndFourth);
        dl_matrix3d_t *fc5 = dl_matrix3d_alloc(1,64,96,1);
        dl_matrix3d_t *fc6 = dl_matrix3d_alloc(1,64,96,1);
        getonet_cut4_fc_0(fc5);
        getonet_cut5_fc_0(fc6);
//not here bool checkMem =heap_caps_check_integrity_all(true);
        dl_matrix3d_t *fcStep3 = dl_matrix3d_alloc(1,64,192,1);
        fcStep3 = dl_matrix3d_concat(fc5,fc6);
        dl_matrix3d_free(fc5);
        dl_matrix3d_free(fc6);
        dl_matrix3d_t *fc7 = dl_matrix3d_alloc(1,64,96,1);
        dl_matrix3d_t *fc8 = dl_matrix3d_alloc(1,64,96,1);
//bool checkMem =heap_caps_check_integrity_all(true);
        getonet_cut6_fc_0(fc7);=
        getonet_cut7_fc_0(fc8);
	dl_matrix3d_t *fcStep4 = dl_matrix3d_alloc(1,64,192,1);
        fcStep4=dl_matrix3d_concat(fc7,fc8);
        dl_matrix3d_free(fc7);
        dl_matrix3d_free(fc8);
	dl_matrix3d_t *fc_lastHalf = dl_matrix3d_alloc(1,64,384,1);
	fc_lastHalf=dl_matrix3d_concat(fcStep3,fcStep4);	
	dl_matrix3d_free(fcStep3);
	dl_matrix3d_free(fcStep4);
//bool checkMem =heap_caps_check_integrity_all(true);

	dl_matrix3d_t *fc_in = dl_matrix3d_alloc(1,64,768,1);
	fc_in=dl_matrix3d_concat(fc_half,fc_lastHalf);
	dl_matrix3d_free(fc_half);
	dl_matrix3d_free(fc_lastHalf);
//bool checkMem =heap_caps_check_integrity_all(true);

printf("\n[ONET] FC INIT!\n");
        dl_matrix3d_t *fc_bias = dl_matrix3d_alloc(1,1,1,64);
        dl_matrix3d_t *fc_out = dl_matrix3d_alloc(1,1,1,64);
        
	getonet_fc_1(fc_bias);

//bool checkMem =heap_caps_check_integrity_all(true);
//printf("\n[ONET] B4 FC \n");        
	//===================
	dl_matrix3dff_fc_with_bias(fc_out,pool3,fc_in,fc_bias);
	//===================
printf("[ONET] AFTER FC\n");
bool checkMem1 =heap_caps_check_integrity_all(true);
//printf("[ONET] POOL3 C: %i H:%i  W: %i",pool3->c,pool3->h,pool3->w);

	dl_matrix3d_free(fc_bias);
	dl_matrix3d_free(pool3);
	dl_matrix3d_free(fc_in);
	
//bool checkMem =heap_caps_check_integrity_all(true);

printf("\n[ONET]  FC DONE!\n\n");

	dl_matrix3d_t *fc_prelu = dl_matrix3d_alloc(1,1,1,64);
	getonet_prelu5_0(fc_prelu);
	
	dl_matrix3d_p_relu(fc_out,fc_prelu);
	dl_matrix3d_free(fc_prelu);

printf("\n[ONET] Done FC prelu\n");
        //===========================================
        // Onet layer 5 --> Scores and Bounding boxes
*/

