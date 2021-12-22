/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    prv_iconv.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/7/15
 *
 * Description:
 *          �ַ�����ת���ڲ������ļ�
 **************************************************************************/

#ifndef __PRV_ICONV_H__
#define __PRV_ICONV_H__

/* ����ת�������� iconv */
typedef size_t (*iconv_fct) (char ** __inbuf,
                      size_t * __inbytesleft,
                      char ** __outbuf,
                      size_t * __outbytesleft);

size_t iconv_ucs2_to_gb2312(char **, size_t *, char **, size_t *);
/*+\NEW\liweiqiang\2013.11.26\����gb2312<->ucs2(ucs2be)����ת��*/
size_t iconv_ucs2be_to_gb2312(char **, size_t *, char **, size_t *);
size_t iconv_gb2312_to_ucs2(char **, size_t *, char **, size_t *);
size_t iconv_gb2312_to_ucs2be(char **, size_t *, char **, size_t *);
/*-\NEW\liweiqiang\2013.11.26\����gb2312<->ucs2(ucs2be)����ת��*/

/*+\NEW\liweiqiang\2013.7.19\����utf8<->ucs2,ucs2be����ת��*/
size_t iconv_utf8_to_ucs2(char **, size_t *, char **, size_t *);

size_t iconv_utf8_to_ucs2be(char **, size_t *, char **, size_t *);

size_t iconv_ucs2_to_utf8(char **, size_t *, char **, size_t *);

size_t iconv_ucs2be_to_utf8(char **, size_t *, char **, size_t *);
/*-\NEW\liweiqiang\2013.7.19\����utf8<->ucs2,ucs2be����ת��*/

#endif/*__PRV_ICONV_H__*/