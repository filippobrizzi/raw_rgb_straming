
#include "zmq.hpp"
#include "x264encoder.h"

#define RGB_SIZE = 640*480*3

int width = 640;
int height = 480;

struct DataPacked{
	uint32_t frame_id_;
	uint32_t color_size_;
	char     data_[RGB_SIZE]; 
}__attribute__((packed));


//Pack the NALs created by x264 into a single packet.
unsigned int pack_rgb_data(DataPacked &data){
	unsigned int tmp_size = 0;
	
	x264_nal_t nal;
	while(x264_encoder_.isNalsAvailableInOutputQueue()){
		nal = x264_encoder_.getNalUnit();
	    memcpy(&(data.data_[tmp_size]), nal.p_payload, nal.i_payload);
		tmp_size += nal.i_payload;
	}
	data.color_size_ = tmp_size;
	//Size of DataPacked after data insert
	return sizeof(uint32_t) * 2 + data.color_size_;
}


int main(){

	char *rgb_buffer = new char[RGB_SIZE];
	DataPacked data;
	x264Encoder x264_encoder;
	x264_encoder.initialize(w, h);

	zmq::context_t context (1);	
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");

	while(true){
		//Any grabber to get raw rgb data from camera.
		camera.getRawData(rgb_buffer);
		
		//Wait for the client to send a request.
		zmq::message_t request;
    	socket.recv (&request);
		
		//Encode the frame with x264
		x264_encoder.encodeFrame(rgb_buffer, RGB_SIZE);
		unsigned int message_size = pack_rgb_data(data);

		//Send the data to the client
		zmq::message_t video_frame (message_size);
        memcpy ((void *) video_frame.data(), &data, message_size);
        socket.send (video_frame);
	}
}

