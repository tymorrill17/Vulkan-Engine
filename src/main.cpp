#include <vk_engine.h>
// Do nothing except call engine functions
// Good place to process command line arguments.
int main(int argc, char* argv[])
{
	VulkanEngine engine;
	engine.init();	
	engine.run();
	engine.cleanup();
	return 0;
}
