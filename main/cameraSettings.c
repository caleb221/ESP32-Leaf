/*
	CAMERA DRIVER
	ESP32-CAM AI-Thinker module
	initialize and configure
	the camera with
		boolean cameraConfig
		--> returns true on success
		--> false on error
	take a picture (get image)
	and provide a space for it in memory
	with
	 	boolean takePic(dl_matrix3du *space for image data)
		--> returns true on sucess
		--> false on fail
		--> image data stored in matrix, assigned as a pointer
			*** MUST free later ***
*/

#include "cameraSettings.h"

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"  
#include "image_util.h"
#include "esp32-hal-psram.h"        // Disable brownour problems
//#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
//#include "driver/rtc_io.h"   // Camera Driver



// define the number of bytes you want to access
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


//static mtmn_config_t mtmn_config = {0};
//static net_config_t pnet_config; 
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

     if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    printf("Camera init failed with error");
    return false;
  }
  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA); 
/*  
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.7;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 4;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.4;
    mtmn_config.o_threshold.candidate_number = 1;
    mtmn_init_config();
*/  
  return true;
}

                   
bool takePic(dl_matrix3du_t *image_matrix_out)
{
  camera_fb_t * pic = NULL;  
  esp_err_t err = ESP_OK;
  // Take Picture with Camera

    //size_t _jpg_buf_len = 0;
    //uint8_t * _jpg_buf = NULL;

  pic = esp_camera_fb_get();  
    if(!pic) 
    { 
      printf("ERR TAKING PICTURE");
       //Serial.println("Camera capture failed");
       return false;
    }
//  size_t pic_len=0;

    if(pic->format == PIXFORMAT_JPEG)
    {
    //MAYBE ADD JPEG CHUNKING HERE?
        jpg_chunking_t jchunk = {0};
        err = frame2jpg_cb(pic, 80/*quality*/, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
  //      pic_len = pic-> len;
        // Serial.print("...JPEG... ");
      //  Serial.print(jpg_encode_stream);
        if(err == ESP_FAIL)
        { 
          printf("ERROR IN JPEG: ");
      //    printf(err);
        }
        //else
        //{
        //            _jpg_buf_len = pic->len;
        //            _jpg_buf = pic->buf;
        //}
    }  
   //allocate array
   
   dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, pic->width, pic->height, 3);
   if(!image_matrix)
   {
    
     dl_matrix3du_free(image_matrix);
     esp_camera_fb_return(pic);
     printf("image matrix allocation failed");
     return false;
      
   }
   
   //get image data into the array
  if(!fmt2rgb888(pic->buf,pic->len,pic->format,image_matrix->item))
  {
    
  //if(!fmt2jpg(image_matrix->item, pic->width*pic->height*3, pic->width, pic->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len))
 // {
    dl_matrix3du_free(image_matrix);
    esp_camera_fb_return(pic);
    printf("FORMAT 2 RGB 888 FAILED");
    return false;
  //}
  } 
 	
	*image_matrix_out = *image_matrix;	
    //free the camera frame for next picture
  	  esp_camera_fb_return(pic);

 /* 
int i=0;
while(i<image_width)
{
  //Serial.print(" ");
//  Serial.print(image_data[i]);  
  
  i++;
}
*/
return true; 
}

