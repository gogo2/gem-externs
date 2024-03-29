/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Calculate the center of gravity of a pixBlock.

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2011 Ricardo Fabbri. labmacambira.sf.net rfabbri@gmail.com
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/
/*-----------------------------------------------------------------

Copyright (c) 2019 Bartosz Sobol https://github.com/gogo2

-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
pix_multiblob2
 tracks multiple blobs in one image

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MULTIBLOB2_H_
#define _INCLUDE__GEM_PIXES_PIX_MULTIBLOB2_H_

#include "Base/GemPixObj.h"
#include <cstdio>


class GEM_EXTERN Blob {
public:

    Blob();

    double xmin() const;

    double xmax() const;

    double ymin() const;

    double ymax() const;

    double xmid() const;

    double ymid() const;

    // the squared diameter of the blob
    double diameter2() const;

    // the diamter
    double diameter() const;

    double angle() {
        double mm_11 = m_11 - xmid() * m_yaccum;
        double mm_02 = m_02 - ymid() * m_yaccum;
        double mm_20 = m_20 - xmid() * m_xaccum;
        return .5 * atan2(2. * mm_11, (mm_20 - mm_02));
    }

    // the angular orientation of the principal axis in radians
    double orientation();

    // the squared distance to another blob
    double distance2(const Blob &b) const;

    // the distance to another blob
    double distance(const Blob &b) const;

    void xmin(double x);

    void xmax(double x);

    void ymin(double y);

    void ymax(double y);

    // area:  moment M_00
    int area;
    // m_xaccum: moment M_10
    // m_yaccum: moment M_01
    double m_xaccum, m_yaccum, m_xyaccum;

    // raw moments for orientation calculation
    double m_11, m_02, m_20;

private :
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
};

class GEM_EXTERN pix_multiblob2 : public GemPixObj {
CPPEXTERN_HEADER(pix_multiblob2, GemPixObj)

public:

    //////////
    // Constructor
    explicit pix_multiblob2(t_float f);

    //Destructor
    ~pix_multiblob2();

    void processImage(imageStruct &image);

    void doProcessing();

    void addToBlobArray(Blob *pblob, int blobNumber);

    void makeBlob(Blob *pb, int x, int y);

    void numBlobsMess(unsigned int blobs);

    void blobMinSizeMess(t_float blobMinSize);

    void blobMaxSizeMess(t_float blobMaxSize);

    void threshMess(t_float thresh);

protected:

    imageStruct m_image;

    int m_blobNumber;
    Blob *m_currentBlobs;

    // the minimum size of a blob (relative to the image)
    t_float m_blobminsize;
    t_float m_blobmaxsize;

    // the minimum value of a pixel to be within a blob
    unsigned char m_threshold;

    // outlets for results
    t_outlet *m_infoOut;

    void blobSizeWarning() const;
};

#endif  // for header file
