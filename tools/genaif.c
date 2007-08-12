/*
	genaif: Generate EPOC32 AIF files, which are used by the EPOC32 shell
	e.g. to display the icon and the program name on the extras bar
	License: GPL.
	Copyright 1999 Rudolf Koenig.
	Copyright 2001 KI-AG
	See AIF format dokumentation on http://koeniglich.de
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* NOTE: we have to write intel byte oder. Fix this if not on native */

#define PUTLONG(X) PutLong(&X, fpout)
#define GETLONG(X) GetLong(&X, fpin)

void GetLong(long* ret_val, FILE *fp)
{
      long val;
      val =  (long) (fgetc(fp) & 0xFF);
      val |= ((long) (fgetc(fp) & 0xFF) << 0x08);
      val |= ((long) (fgetc(fp) & 0xFF) << 0x10);
      val |= ((long) (fgetc(fp) & 0xFF) << 0x18);
      *ret_val = val;
}

void PutLong(long* val, FILE *fp)
{
      fputc(*val & 0xFF, fp);
      fputc((*val >> 0x08) & 0xFF, fp);
      fputc((*val >> 0x10) & 0xFF, fp);
      fputc((*val >> 0x18) & 0xFF, fp);
}



static char *Usage =
	"Usage: genaif [-u] UID3 aifspecfile app.aif\n"
	"   or: genaif -print-checksum UID1 UID2 UID3 csum.new\n"
	"   or: genaif -change-app-uid3 UID3 old.app new.app\n"
	"\n"
	"# Sample aifspecfile:\n"
	"# Mbmfile contains a number of icon/mask pairs\n"
	"mbmfile=app.mbm\n"
	"ELangEnglish=Application\n"
	"ELangGerman=Applikation\n"
	"hidden=0\n"
	"embeddability=0\n"
	"newfile=1\n";

static char *langs[] = {
	"ELangTest",
	"ELangEnglish",
	"ELangFrench",
	"ELangGerman",
	"ELangSpanish",
	"ELangItalian",
	"ELangSwedish",
	"ELangDanish",
	"ELangNorwegian",
	"ELangFinnish",
	"ELangAmerican",
	"ELangSwissFrench",
	"ELangSwissGerman",
	"ELangPortuguese",
	"ELangTurkish",
	"ELangIcelandic",
	"ELangRussian",
	"ELangHungarian",
	"ELangDutch",
	"ELangBelgianFlemish",
	"ELangAustralian",
	"ELangBelgianFrench",
	"ELangAustrian",
	"ELangNewZealand",
	0
};


int
docrc16_1(crc, c)
  unsigned int crc;
  unsigned char c;
{
  unsigned int acc, i;

  acc = crc;
  acc ^= c << 8;
  for(i = 0; i < 8; i++)
    if(acc & 0x8000)
      {
	acc <<= 1;
	acc ^= 0x1021;
      }
    else
      {
	acc <<= 1;
      }
  return acc & 0xffff;
}

unsigned long
uidcsum(unsigned long u[3])
{
  unsigned int i, crc1 = 0, crc2 = 0;
  
  for(i = 0; i < 3; i++)
    {
      crc1 = docrc16_1(crc1, (u[i] >>  0) & 0xff);
      crc2 = docrc16_1(crc2, (u[i] >>  8) & 0xff);
      crc1 = docrc16_1(crc1, (u[i] >> 16) & 0xff);
      crc2 = docrc16_1(crc2, (u[i] >> 24) & 0xff);
    }
  return (crc2 << 16) | crc1;
}

int
writeAif(int uni, char *cuid, char *finname, char *foutname)
{
  char *s, *arg, buf[1024], *caps[128], *mbmfile;
  int len, rd, idx, no, capno, embed, hidden, new, cappos[128];
  unsigned char outchar, caplang[128];
  unsigned long l[3], uid;
  FILE *fpin, *fpout, *fpfile;

  if(!(fpfile = fopen(finname, "rb")))
    {
      perror(finname);
      return 1;
    }
  if(!(fpout = fopen(foutname, "wb")))
    {
      perror(foutname);
      return 1;
    }

  uid = strtol(cuid, 0, 0);
  rd = no = hidden = embed = capno = new = 0;
  mbmfile = 0;
  while(fgets(buf, sizeof(buf), fpfile))
    {
      no++;
      if(!*buf || *buf == '#')
        continue;
      if(!(s = strtok(buf, "=")) || !(arg = strtok(0, "\n\r")))
        {
	  fprintf(stderr, "Bogus data in line %d\n\n%s", no, Usage);
	  return 1;
	}
      if(!strcasecmp(s, "mbmfile"))
	mbmfile = strdup(arg);
      else if(!strcasecmp(s, "hidden"))
        hidden = strtol(arg, 0, 0);
      else if(!strcasecmp(s, "embeddability"))
        embed = strtol(arg, 0, 0);
      else if(!strcasecmp(s, "newfile"))
        new = strtol(arg, 0, 0);
      else {
        for(idx = 0; langs[idx]; idx++)
	  if(!strcasecmp(langs[idx], s))
	    break;
	if(capno > 128)
	  {
	    fprintf(stderr, "Sorry, too man captions\n");
	    return 1;
	  }
	if(langs[idx])
	  {
	    if(strlen(arg) > 63)
	      {
	        fprintf(stderr, "Sorry, caption in line %d too long\n", no);
		return 1;
	      }
	    rd += strlen(arg) + 1;
	    caps[capno] = strdup(arg);
	    caplang[capno++] = idx;
	    continue;
	  }
	fprintf(stderr, "Unknown keyword in line %d\n\n%s", no, Usage);
	return 1;
      }
    }
  
  if(uid == 0)
    {
      fprintf(stderr, "Uid is missing\n%s", Usage);
      return 0;
    }
  
  l[0] = 0x10000037; PUTLONG(l[0]);
  l[1] = uni ? 0x10003a38 : 0x1000006a;
  PUTLONG(l[1]);
  l[2] = uid;        PUTLONG(l[2]);

  no = uidcsum(l);
  PUTLONG(no);

  if (!mbmfile)
    {
    no = 0;
    PUTLONG(no);
    fclose(fpfile);
    fclose(fpout);
    return 0;
    }

  /* Mbmfile reading */
  if(!(fpin = fopen(mbmfile, "rb")))
    {
      perror(mbmfile);
      return 1;
    }
  GETLONG(no);
  GETLONG(uid);
  if(no != 0x10000037 || uid != 0x10000042)
    {
      fprintf(stderr, "Unknown MBM file, sorry, no output\n");
      return 1;
    }
  GETLONG(no);	/* Ignored */
  GETLONG(no);	/* Ignored */
  GETLONG(no);	/* This is the trailer offset */

  idx = no + rd;
  PUTLONG(idx);	/* This is the output offset */

  /* Copy the MBM file */
  rd = 1;
  for(idx = 20; idx < no && rd != 0; idx += rd)
    {
      rd = (no-idx) > sizeof(buf) ? sizeof(buf) : no-idx;
      rd = fread(buf, 1, rd, fpin);
      fwrite(buf, 1, rd, fpout);
    }
  if(!rd)
    {
      fprintf(stderr, "Sorry, mbm file too short!\n");
      return 1;
    }

  /* write out the captions */
  rd = no;
  for(idx = 0; idx < capno; idx++)
    {
      cappos[idx] = rd;
      len = strlen(caps[idx]);
      outchar = len * 4 + (uni ? 0 : 2);
      putc(outchar, fpout);
      fwrite(caps[idx], len, 1, fpout);
      rd += len + 1;
    }
  
  /* Trailer time!! */
  /* First the caption offsets */
  outchar = 2 * capno;
  putc(outchar, fpout);
  for(idx = 0; idx < capno; idx++)
    {
      PUTLONG(cappos[idx]);
      putc(caplang[idx], fpout);
      putc(0, fpout);	/* This fails if there are more than 255 langs */
    }
  
  /* Now the Picture offsets */
  GETLONG(no);
  if(no % 2 != 0 || !no)
    {
      fprintf(stderr, "Sorry, we need some icon PAIRS (image/mask)\n");
      return 1;
    }

  outchar = no;		/* Hope this works every time... */
  putc(outchar, fpout);

  for(idx = 0; idx < no; idx += 2)
    {
      GETLONG(rd);	/* Offsets are in mbm and aif the same */
      PUTLONG(rd);
      len = ftell(fpin);
      fseek(fpin, rd, SEEK_SET);
      GETLONG(rd);	/* Length of chunk, ignored */
      GETLONG(rd);	/* Headerlength, ignored */
      GETLONG(rd);	/* X-Size! This is what we are looking for */

      outchar = rd & 0xff; putc(outchar, fpout);	/* Write it as short */
      outchar = (rd & 0xff00) >> 8; putc(outchar, fpout);

      fseek(fpin, len, SEEK_SET);
      GETLONG(rd);
    }

  no = 0x00000001;
  PUTLONG(no);
  PUTLONG(embed);
  PUTLONG(new);
  PUTLONG(hidden);

  fclose(fpfile);
  fclose(fpin);
  fclose(fpout);
  
  return 0;
}

int
changeAppUid(char *uid3, char *finname, char *foutname)
{
  unsigned long l[3], id, oldid, count = 0;
  FILE *fpin, *fpout;

  if(!(fpin = fopen(finname, "rb")))
    {
      perror(finname);
      return 1;
    }
  if(!(fpout = fopen(foutname, "wb")))
    {
      perror(foutname);
      return 1;
    }
  GETLONG(l[0]);
  GETLONG(l[1]);
  GETLONG(l[2]);
  GETLONG(id);
  if(!(l[0] == 0x10000079 && l[1] == 0x1000006c) &&  /* EPOCR5 */
     !(l[0] == 0x1000007a && l[1] == 0x100039ce))    /* EPOCR6 */
    {
      fprintf(stderr, "Not an app file, aborting\n");
      return 1;
    }
  oldid = l[2];
  l[2] = strtol(uid3, 0, 0);

  fprintf(stderr, "Changing UID from %#lx to %#lx\n", oldid, l[2]);
  
  id = uidcsum(l);
  PUTLONG(l[0]);
  PUTLONG(l[1]);
  PUTLONG(l[2]);
  PUTLONG(id);
  count++;
  while(!feof(fpin))
    {
      GETLONG(id);
      if(id == oldid)
        count++, id = l[2];
      PUTLONG(id);
    }
  fclose(fpin);
  fclose(fpout);

  if(count != 2)
    {
      fprintf(stderr, "Changed UID %ld times (instead of 2).\n", count);
      fprintf(stderr, "The output is very probably unusable.\n");
      return 1;
    }
  return 0;
}

int
main(int ac, char *av[])
{
  if(ac == 5 && !strcmp(av[1], "-print-checksum"))
    {
      unsigned long l[3];
      l[0] = strtol(av[2], 0, 0);
      l[1] = strtol(av[3], 0, 0);
      l[2] = strtol(av[4], 0, 0);
      printf("%#08lx\n", uidcsum(l));
      return 0;
    }
  else if(ac == 5 && !strcmp(av[1], "-change-app-uid3"))
    return changeAppUid(av[2], av[3], av[4]);
  else 
    {
      int uni = 0;
      if(ac > 1 && !strcmp(av[1], "-u"))
	uni = 1, ac--, av++;
      if(ac == 4)
        return writeAif(uni, av[1], av[2], av[3]);
    }

  fprintf(stderr, "%s", Usage);
  return 0;
}
