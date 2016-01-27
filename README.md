Un libreria molto incasinata di utilities per OpenCL.
Praticamente tutto si basa sulla documentazione di OpenCL 1.2.

## Dependencies

- [MCLabUtils](https://bitbucket.org/mclab/mclabutils).
- Le utilities generiche che uso per il C, da [qui](https://github.com/asmeikal/C-utils).
- `stb_image.h` e `stb_image_write.h` da [qui](https://github.com/nothings/stb).

Per "semplicità" mantengo dei file in una cartella comune, in cui sono definite delle variabili con i percorsi per trovare le librerie.
I file sono:
- `mclabutils.mk`
- `mlutils.mk`
- `stb_lib.mk`

Le variabili che definiscono sono, rispettivamente:
- `MCLAB_INCLUDE` e `MCLAB_LIB` per gli header e per la cartella contenente la libreria compilata.
- `ML_INCLUDE` e `ML_LIB`, come sopra.
- `STB_DIR` per la cartella in cui si trovano gli header di stb.

## Contents

- `mlclut.c`: funzioni abbondantemente generiche.
- `mlclut_descriptions.c`: funzioni per descrivere/stampare tipi enumerati di OpenCL.
- `mlclut_images.c`: funzioni per aprire e salvare immagini.

La funzione per aprire immagini usa come channel\_order uno fra `CL_R`, `CL_RA`, e `CL_RGBA`.
La funzione per salvare immagini salva in formato PNG.

