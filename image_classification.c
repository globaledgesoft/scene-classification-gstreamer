#include "classification.h"
#include <string.h>

GstElement *pipeline;

void sigintHandler(int unused)
{
	g_print("Sending EOS stream to the pipeline\n");
	gst_element_send_event(pipeline, gst_event_new_eos());
	g_print("pipeline got EOS stream\n");
}

int main(int argc, char *argv[])
{
	char modelPath[100];
	char label[100];
	char sysType[5];

        strcpy(modelPath, argv[2]);                //  tflite model file 
        strcpy(label, argv[3]);                    //  labelmap file  

	GstElement *source, *filter, *tflite, *overlay, *enc, *parse, *mux, *sink;
	GstCaps *caps;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn state;
	
	gst_init(&argc, &argv);

	pipeline = gst_pipeline_new("pipeline");
	
	source = gst_element_factory_make ("qtiqmmfsrc", "source");
	filter = gst_element_factory_make ("capsfilter", "filter");
	tflite = gst_element_factory_make ("qtimletflite", "tflite");
	overlay = gst_element_factory_make ("qtioverlay", "overlay");
	enc = gst_element_factory_make ("omxh264enc", "enc");
	parse = gst_element_factory_make ("h264parse", "parse");
	mux = gst_element_factory_make ("mp4mux", "mux");
	sink = gst_element_factory_make ("filesink", "sink");

	if (!pipeline || !source || !filter || !tflite || !overlay || !enc || !parse || !mux || !sink) {
		g_printerr("Elements could not be created\n");
	}

	caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "NV12", "width", G_TYPE_INT, WIDTH, "height", G_TYPE_INT, HEIGHT, "framerate", GST_TYPE_FRACTION, FPS, 1, "camera", G_TYPE_INT, CAMERA_ID, NULL);
	
	g_object_set (source, "name", "qmmf", NULL);
	g_object_set(source, "ldc",TRUE,  NULL);
	g_object_set (filter, "caps", caps, NULL);

	gst_bin_add_many (GST_BIN(pipeline), source, filter, tflite, overlay, enc, parse, mux, sink, NULL);
	if (!gst_element_link_many(source, filter, tflite, overlay, enc, parse, mux, sink, NULL)) {
		g_printerr("Elements could not be linked\n");
	}
	
	if (!(strcmp(argv[1],"default"))) {
	g_object_set(tflite, "delegate", argv[1], "model", argv[2], "labels", argv[3], "postprocessing", "classification", NULL);
	}
	if (!(strcmp(argv[1], "dsp"))) {
	g_object_set(tflite, "delegate", argv[1], "model", argv[2], "labels", argv[3], "postprocessing", "classification", NULL);
	}
	
	g_object_set(sink, "location", "/data/c610/video.mp4", NULL);

	g_print("Setting pipeline to the playing state\n");
	state = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (state = GST_STATE_CHANGE_FAILURE) {
		g_printerr("Could not set pipeline to the playing state\n");
	}

	bus = gst_element_get_bus (pipeline);
	
	signal(SIGINT, sigintHandler);
	
	msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

	if (msg != NULL) {
		GError *err;
		gchar *debug_info;

		switch (GST_MESSAGE_TYPE (msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error (msg, &err, &debug_info);
				g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
				g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
				g_clear_error (&err);
				g_free (debug_info);
				break;
		}
		gst_message_unref (msg);
	}
	
	g_print("Freeing resources\n");
	gst_object_unref(bus);
	gst_caps_unref(caps);
	g_print("Setting pipeline to the NULL state\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);

	return 0;
}
