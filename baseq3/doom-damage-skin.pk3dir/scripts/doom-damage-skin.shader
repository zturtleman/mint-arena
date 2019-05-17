// Doom damage skins

// Single shader damage skin (entity alpha)
// mesh,models/players/doom/doom00

models/players/doom/doom00
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/doom_statue.tga
		alphaGen entity
		blendFunc blend
	}
}

models/players/doom/doom_f00
{
	{
		map models/players/doom/doom_f.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/doom_f_statue.tga
		alphaGen entity
		blendFunc blend
	}
}

// Multi-shader damage skin:
// mesh,models/players/doom/doom01,models/players/doom/doom02,models/players/doom/doom03,models/players/doom/doom04,models/players/doom/doom05,models/players/doom/doom06,models/players/doom/doom07,models/players/doom/doom08,models/players/doom/doom09,models/players/doom/doom10

// 91+ health
models/players/doom/doom01
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
}

// 81 - 90 health
models/players/doom/doom02
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/red.tga
		alphaGen const 0.3
		blendFunc blend
	}
}

// 71 - 80 health
models/players/doom/doom03
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/red.tga
		alphaGen const 0.4
		blendFunc blend
	}
}

// 61 - 70 health
models/players/doom/doom04
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/red.tga
		alphaGen const 0.6
		blendFunc blend
	}
}

// 51 - 60 health
models/players/doom/doom05
{
	{
		map models/players/doom/doom.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/red.tga
		alphaGen const 0.8
		blendFunc blend
	}
}

// 41 - 50 health
models/players/doom/doom06
{
	{
		map models/players/doom/red.tga
		rgbGen lightingDiffuse
	}
}

// 31 - 40 health
models/players/doom/doom07
{
	{
		map models/players/doom/red.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/doom_statue.tga
		alphaGen const 0.25
		blendFunc blend
	}
}

// 21 - 30 health
models/players/doom/doom08
{
	{
		map models/players/doom/red.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/doom_statue.tga
		alphaGen const 0.5
		blendFunc blend
	}
}

// 11 - 20 health
models/players/doom/doom09
{
	{
		map models/players/doom/red.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/doom/doom_statue.tga
		alphaGen const 0.75
		blendFunc blend
	}
}

// 0 - 10 health
models/players/doom/doom10
{
	{
		map models/players/doom/doom_statue.tga
		rgbGen lightingDiffuse
	}
}
