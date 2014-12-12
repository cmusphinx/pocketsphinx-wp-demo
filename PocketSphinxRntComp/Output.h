#pragma once

namespace PocketSphinxRntComp
{

	public ref class Output sealed
	{
	public:
		static void WriteLine(Platform::String^ message);
		static void Write(Platform::String^ message);

	private:
		Output();
	};

}