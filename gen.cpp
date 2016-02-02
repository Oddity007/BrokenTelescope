#include <stdio.h>
/*#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>*/
#include "lua.hpp"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtc/random.hpp"

#include <SFML/Graphics.hpp>
//#include <SFML/Image.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"


#define PUBLIC_C_VISIBILITY extern "C" __attribute__ ((visibility ("default")))


static int Hello(lua_State* L)
{
	puts("Hello");
	return 0;
}

PUBLIC_C_VISIBILITY int libinit(lua_State* L)
{
	lua_register(L, "Hello", Hello);
	return 0;
}

static sf::Image Process_old(const sf::Image& in, const sf::Texture& brushTexture)
{
	sf::Vector2u size = in.getSize();
	
	auto sampleAt = [&](const float* luminances, int x, int y)
	{
		while (x < 0)
			x += (int) size.x;
		while (y < 0)
			y += (int) size.y;
		x %= (int) size.x;
		y %= (int) size.y;
		return luminances[y * size.x + x];
	};
	
	//Init the data
	
	float* originalLuminances = new float[size.x * size.y];
	
	for (unsigned int y = 0; y < size.y; y++)
	for (unsigned int x = 0; x < size.x; x++)
	{
		sf::Color inColor = in.getPixel(x, y);
		originalLuminances[y * size.x + x] = glm::dot(glm::vec3(0.1, 0.6, 0.3), glm::vec3(inColor.r, inColor.g, inColor.b) * glm::vec3(1.0f/255.0f));
		//out.setPixel(x, y, sf::Color(c * 255, c * 255, c * 255));
	}
	
	//x filter
	
	float* xSobels = new float[size.x * size.y];
	
	for (int y = 0; y < size.y; y++)
	for (int x = 0; x < size.x; x++)
	{
		float r = 0;
		r += sampleAt(originalLuminances, x + 1, y + 1) - sampleAt(originalLuminances, x - 1, y + 1);
		r += 2.0f * sampleAt(originalLuminances, x + 1, y + 0) - 2.0f* sampleAt(originalLuminances, x - 1, y + 0);
		r += sampleAt(originalLuminances, x + 1, y - 1) - sampleAt(originalLuminances, x - 1, y - 1);
		xSobels[y * size.x + x] = r;
	}
	
	
	float* ySobels = new float[size.x * size.y];
	
	for (int y = 0; y < size.y; y++)
	for (int x = 0; x < size.x; x++)
	{
		float r = 0;
		r += sampleAt(xSobels, x + 1, y - 1) - sampleAt(xSobels, x + 1, y + 1);
		r += 2.0f * sampleAt(xSobels, x + 0, y - 1) - 2.0f* sampleAt(xSobels, x + 0, y + 1);
		r += sampleAt(xSobels, x - 1, y - 1) - sampleAt(xSobels, x - 1, y + 1);
		ySobels[y * size.x + x] = r;
	}
	
	sf::RenderTexture renderTexture;
	if (not renderTexture.create(size.x, size.y))
		abort();
	
	renderTexture.clear();
	//renderTexture.draw()
	
	/*sf::Image out;
	out.create(size.x, size.y);
	for (unsigned int y = 0; y < size.y; y++)
	for (unsigned int x = 0; x < size.x; x++)
	{
		//sf::Color inColor = in.getColor(x, y);
		//originalLuminances[y * size.x + x] = glm::dot(glm::vec3(0.1, 0.6, 0.3), glm::vec3(inColor.r, inColor.g, inColor.b) * glm::vec3(1.0f/255.0f));
		float c = sampleAt(ySobels, x, y);
		c = 1.0f - glm::clamp(c, 0.0f, 1.0f);
		out.setPixel(x, y, sf::Color(c * 255, c * 255, c * 255));
	}*/
	//return out;
	
	sf::Sprite brush(brushTexture);
	float brushRadius = 10.0f;
	
	auto approximateGradientAt = [&](const float* luminances, int x, int y)
	{
		const int maximumSampleCount = 16;
		
		glm::vec2 gradient(0, 0);
		
		for (int i = 0; i < maximumSampleCount; i++)
		{
			glm::vec2 d = glm::diskRand(glm::linearRand(1.44f, brushRadius));
			gradient += (sampleAt(luminances, int(x + d.x), int(y + d.y)) - sampleAt(luminances, int(x + d.x), int(y + d.y))) * d;
		}
		
		return gradient;
	};
	
	auto approximateAverageAt =
	[&](const float* luminances, int x, int y)
	{
		const int maximumSampleCount = 16;
		
		float average = 0;
		
		for (int i = 0; i < maximumSampleCount; i++)
		{
			glm::vec2 d = glm::diskRand(glm::linearRand(1.44f, brushRadius));
			average += sampleAt(luminances, int(x + d.x), int(y + d.y)) / float(maximumSampleCount);
		}
		
		return average;
	};
	
	const size_t maximumIterationCount = 100000;
	glm::vec2 startPosition = glm::linearRand(glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
	for (size_t i = 0; i < maximumIterationCount; i++)
	{
		//startPosition = glm::linearRand(glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
		if (approximateAverageAt(ySobels, startPosition.x, startPosition.y) < 1)
		{
			startPosition = glm::linearRand(glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
			continue;
		}
		
		brush.setPosition(startPosition.x, startPosition.y);
		brush.setRotation(glm::linearRand(0.0f, 360.0f));
		renderTexture.draw(brush);
		
		//glm::vec2 gradient = approximateGradientAt(ySobels, startPosition.x, startPosition.y);
		glm::vec2 gradient = glm::diskRand(glm::linearRand(1.44f, brushRadius));
		//if (glm::dot(gradient, gradient) < 1)
		//	startPosition = glm::linearRand(glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
		//startPosition += glm::normalize(gradient);
		startPosition += gradient;
		
		if (approximateAverageAt(ySobels, startPosition.x, startPosition.y) < 1)
			startPosition = glm::linearRand(glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
		
		startPosition = glm::clamp(startPosition, glm::vec2(0.0f), glm::vec2(float(size.x), float(size.y)));
	}
	
	delete[] xSobels;
	delete[] originalLuminances;
	delete[] ySobels;
	
	return renderTexture.getTexture().copyToImage();
}

int main_old(void)
{
	sf::Image originalImage;
	originalImage.loadFromFile("/Volumes/ODDHFPSMINI/TestingImages/00202/0202014.jpg");
	
	/*sf::RenderTexture renderTexture;
	if (not renderTexture.create(512, 512))
		return 0;
	renderTexture.clear();*/
	//renderTexture.draw();

	//renderTexture.getTexture().copyToImage().saveToFile("temp/output.png");
	
	sf::Image brushImage;
	const int brushSize = 8;
	brushImage.create(brushSize, brushSize);
	for (int i = 0; i < 16 * brushSize; i++)
	{
		glm::vec2 p = glm::diskRand(brushSize * 0.5f) + glm::vec2(brushSize * 0.5f);
		glm::ivec2 ip = glm::clamp(glm::ivec2(p.x, p.y), glm::ivec2(0), glm::ivec2(brushSize - 1));
		brushImage.setPixel(ip.x, ip.y, sf::Color(255, 255, 255));
	}
	
	brushImage.createMaskFromColor(sf::Color(0, 0, 0));
	
	
	sf::Texture brushTexture;
	brushTexture.loadFromImage(brushImage);
	Process_old(originalImage, brushTexture).saveToFile("temp/output.png");
	//brushImage.saveToFile("temp/output.png");
	
	return 0;
}



/*static sf::Image GenerateWorldMap(int width, int height)
{
	auto indexAt = [&](int x, int y)
	{
		while (x < 0)
			x += width;
		while (y < 0)
			y += height;
		x %= width;
		y %= height;
		return y * width + x;
	};
	float* elevations = new float[width * height];
	
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++)
	{
		elevations[indexAt(x, y)] = 0;
	}
	
	for (int i = 0; i < 100; i++)
	{
		elevations[indexAt(x, y)] = 0;
	}
	
	delete[] elevations;
	sf::Image out;
	out.create(width, height);
	
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++)
	{
		glm::vec3 c = glm::mix(glm::vec3(128, 128, 255), glm::vec3(64, 64, 255), y / float(height));
		c = glm::clamp(c, glm::vec3(0), glm::vec3(1));
		out.setPixel(x, y, Color(c.x * 255, c.y * 255, c.z * 255));
	}
	
	return out;
}*/

static sf::Image ResizeImage(const sf::Image& in, unsigned int width, unsigned int height)
{
	sf::RenderTexture renderTexture;
	if (not renderTexture.create(width, height))
		abort();
	renderTexture.clear();
	renderTexture.setSmooth(true);
	assert(renderTexture.isSmooth());
	
	sf::Texture texture;
	texture.loadFromImage(in);
	sf::Sprite sprite(texture);
	sprite.setScale(float(width) / in.getSize().x, float(height) / in.getSize().y);
	renderTexture.draw(sprite);

	return renderTexture.getTexture().copyToImage();
}

static sf::Image BlurImage(const sf::Image& in, glm::ivec2 d)
{
	int width = in.getSize().x;
	int height = in.getSize().y;
	
	auto sampleAt = [&](int x, int y)
	{
		while (x < 0)
			x += width;
		while (y < 0)
			y += height;
		x %= width;
		y %= height;
		sf::Color c = in.getPixel(x, y);
		return glm::vec3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
	};
	
	sf::Image out;
	out.create(width, height);
	for (int y = 0; y < height; y++)
	for (int x = 0; x < width; x++)
	{
		glm::vec3 c(0);
		c += 0.06136 * sampleAt(x - 2 * d.x, y - 2 * d.y);
		c += 0.24477 * sampleAt(x - d.x, y - d.y);
		c += 0.387741f * sampleAt(x, y);
		c += 0.24477 * sampleAt(x + d.x, y + d.y);
		c += 0.06136 * sampleAt(x + 2 * d.x, y + 2 * d.y);
		/*for (int i = - 16; i <= 16; i++)
		{
			c += ((i ? (1 / float(i * i)) : 1) / 2.0f) * sampleAt(x + i * d.x, y + i * d.y);
		}*/
		c = glm::clamp(c, glm::vec3(0), glm::vec3(1));
		out.setPixel(x, y, sf::Color(c.x * 255, c.y * 255, c.z * 255));
	}
	return out;
}

static sf::Image GenerateTelescopePicture(int size)
{
	sf::Image out;
	out.create(size, size);
	
	glm::vec3 noiseSamplePositionScale = glm::linearRand(glm::vec3(0.1f), glm::vec3(16.0f));
	glm::vec3 noiseSamplePositionBias = glm::sphericalRand(glm::linearRand(0.1f, float(size / 2)));
	
	for (int y = 0; y < size; y++)
	for (int x = 0; x < size; x++)
	{
		glm::vec3 c = glm::linearRand(glm::vec3(0), glm::vec3(1));
		glm::vec3 noiseSamplePosition(x / float(size), y / float(size), 0);
		noiseSamplePosition = noiseSamplePositionScale * noiseSamplePosition;
		noiseSamplePosition += noiseSamplePositionBias;
		c *= glm::mix(0.25f, 1.0f, glm::clamp(stb_perlin_noise3(noiseSamplePosition.x, noiseSamplePosition.y, noiseSamplePosition.z, 0, 0, 0), 0.0f, 1.0f));
		c = glm::clamp(c, glm::vec3(0), glm::vec3(1));
		out.setPixel(x, y, sf::Color(c.x * 255, c.y * 255, c.z * 255));
	}
	
	int starSize = 32;
	
	//ResizeImage(ResizeImage(out, 32, 32), size, size)
	
	out = ResizeImage(out, 32, 32);
	
	glm::vec3 starColor = glm::linearRand(glm::vec3(0.5, 0.2, 0.2), glm::vec3(1));
	
	size_t dotCount = glm::linearRand(1, 32) * starSize;
	
	for (int i = 0; i < dotCount; i++)
	{
		glm::vec2 d = glm::diskRand(starSize * 0.5f);
		glm::vec2 p = d + glm::vec2(starSize * 0.5f);
		glm::ivec2 ip = glm::clamp(glm::ivec2(p.x, p.y), glm::ivec2(0), glm::ivec2(starSize - 1));
		glm::vec3 c = (1.0f - glm::length(d) / float(starSize * 0.5)) * starColor;
		out.setPixel(ip.x, ip.y, sf::Color(c.x * 255, c.y * 255, c.z * 255));
	}
	
	for (size_t i = 0; i < 1; i++)
		out = BlurImage(BlurImage(out, glm::ivec2(1, 0)), glm::ivec2(0, 1));
	out = ResizeImage(out, size, size);
	
	for (size_t i = 0; i < 10; i++)
		out = BlurImage(BlurImage(out, glm::ivec2(1, 0)), glm::ivec2(0, 1));
	
	return out;
}

int main(void)
{
	srand(time(NULL));
	GenerateTelescopePicture(512).saveToFile("temp/output.png");
	return 0;
}
