#include <service/service.h>

int main(int argc, char* argv[])
{
	std::vector<module_desc> modules{{"editor", "editor"}};

	service app(argc, argv);

    //for(int i = 0; i < 3; ++i)
	{

		if(!app.load(modules))
		{
			return -1;
		}

        if(!app.init())
        {
            return -1;
        }

		bool run = true;

		while(run)
		{
			run = app.process();
		}

		if(!app.unload())
		{
			return -1;
		}
	}
	return 0;
}
