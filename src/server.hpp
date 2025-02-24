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
	"Content-Type: audio/aac\r\n\r\n", "Content-Type: video/x-msvideo\r\n\r\n",
	"Content-Type: image/bmp\r\n\r\n", "Content-Type: text/css\r\n\r\n",
	"Content-Type: image/gif\r\n\r\n", "Content-Type: image/vnd.microsoft.icon\r\n\r\n",
	"Content-Type: text/javascript\r\n\r\n", "Content-Type: application/jason\r\n\r\n",
	"Content-Type: video/mpt\r\n\r\n", "Content-Type: font/otf\r\n\r\n",
	"Content-Type: image/png\r\n\r\n", "Content-Type: application/x-httpd-php\r\n\r\n",
	"Content-Type: application/rtf\r\n\r\n", "Content-Type: image/svg+xml\r\n\r\n",
	"Content-Type: text/plain\r\n\r\n", "Content-Type: video/webm\r\n\r\n",
	"Content-Type: video/webp\r\n\r\n", "Content-Type: font/woff\r\n\r\n",
	"Content-Type: font/woff2\r\n\r\n", "Content-Type: applicatiion/zip\r\n\r\n",
	"Content-Type: text/html\r\n\r\n", "Content-Type: image/jpeg\r\n\r\n",
	"Content-Type: image/jpg\r\n\r\n", "Content-Type: \r\n\r\n",
	
}
