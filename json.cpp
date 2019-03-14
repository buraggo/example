#include <iostream>
#include <jansson.h>
#include <curl/curl.h>
#include <cstring>
#define BUFFER_SIZE (256 * 1024)
#define URL_SIZE 256
using namespace std;

struct write_result{
	char *data;
	int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream){
	struct write_result *result = (struct write_result *)stream;
	if(result->pos + size * nmemb >= BUFFER_SIZE - 1) {
		fprintf(stderr, "ERROR: too small buffer\n");
		return 0;
	}
	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;
	return size * nmemb;
}

static char *request(const char *url){
		CURL *curl = NULL;
		CURLcode status;
		struct curl_slist *headers = NULL;
		char *data = NULL;
		long code;
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		data = (char*) malloc(BUFFER_SIZE);
		struct write_result write_result = {
			.data = data,
			.pos = 0
		};
		curl_easy_setopt(curl, CURLOPT_URL, url);
		headers=curl_slist_append(headers, "User-Agent: MMA");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
		status = curl_easy_perform(curl);
		if(status != 0) {
			fprintf(stderr, "ERROR: unable to request data from %s:\n", url);
			fprintf(stderr, "%s\n", curl_easy_strerror(status));
			goto error;
		}
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if(code != 200) {
			fprintf(stderr, "ERROR: server responded with code %ld\n", code);
			goto error;
		}
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		curl_global_cleanup();
		data[write_result.pos] = '\0';
		return data;
		
	error:
		if(data) free(data);
		if(curl) curl_easy_cleanup(curl);
		if(headers) curl_slist_free_all(headers);
		curl_global_cleanup();
		return NULL;
}

class Fetchinfo{
	private:
		char *text;
		json_t *root;
		json_error_t error;

	public:
		string Country, Token;
		json_t *main, *temp, *pressure, *wind, *deg, *speed;
		string URL;
		
		void Jansson_Init(){
			cout<<"function jansson_init OK"<<endl;
			text=request(URL.c_str());
			root=json_loads(text, 0, &error);
			free(text);
			main=json_object_get		(root,"main");
				temp=json_object_get	(main,"temp");
				pressure=json_object_get(main,"pressure");
			wind=json_object_get		(root,"wind");
				deg=json_object_get		(wind,"deg");
				speed=json_object_get	(wind,"speed");
			cout<<"##################"<<endl<<"Jansson Parse:"<<endl<<"\tTemp:"<<json_number_value(temp)<<endl<<"\tPress:"<<json_number_value(pressure)<<endl<<"\tDeg:"<<json_number_value(deg)<<endl<<"\tSpeed:"<<json_number_value(speed)<<endl<<"##################"<<endl;
		}
		
		void Jansson_Dstr(){
			json_decref(root);
			cout<<"function jansson_destruct OK"<<endl;
		}
};
int main(){
	Fetchinfo FI;
		FI.URL="http://api.openweathermap.org/data/2.5/weather?q=Moscow&appid=c714b89eb55d5fb6aa02928515b53b84";
		cout<<"URL: "<<FI.URL<<endl;
		FI.Jansson_Init();
		FI.Jansson_Dstr();
return 0;
}
