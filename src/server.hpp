#include <string.h>
#include <string>


typedef enum{
	HTTP_HEADER,
	BAD_REQUEST,
	NOT_FOUND
}messageType;

std::string Message[] = {
	"Http/1.1 200 ok\r\n",
	"Http/1.0 400 Bad requrest \r\n Content-Type:text/html\r\n\r\n <!doctype html><html><body> System is busy right now </body></html>",
	"Http/1.0 404 File not found \r\n Content-Type:text/html\r\n\r\n <!doctype html><html><body> The requested file does not exist on this server </body></html> "
};

std::string fileExtention[]=
{
	"aac", "avi", "bmp","gif", "ico", "js","json",
	"mp3", "mp4","otf", "png", "php","rtf", "svg", "txt"," webm","webp", "woff",
	"zip", "html","jpeg", "jpg"




};

std::string ContentType[]=
{
	"Content-Type",
}
