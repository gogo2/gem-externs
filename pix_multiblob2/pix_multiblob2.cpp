////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// pix_multiblob2
// based on (c) 2004, Jakob Leiner & Theresa Rienmüller
// and stack-based code from animal.sf.net which is
// (c) Ricardo Fabbri labmacambira.sf.net
//
// modifications against pix_multiblob Bartosz Sobol
//
/////////////////////////////////////////////////////////


#include <cassert>
#include "pix_multiblob2.h"
#include "Utils/Functions.h"
#include "gem_utils/pstk.h"

////////////////////////

CPPEXTERN_NEW_WITH_ONE_ARG(pix_multiblob2, t_floatarg, A_DEFFLOAT)

/*------------------------------------------------------------

pix_multiblob2

------------------------------------------------------------*/

/*------------------------------------------------------------

Constructor
initializes the pixBlocks and pixBlobs

------------------------------------------------------------*/
pix_multiblob2::pix_multiblob2(t_floatarg f) :
        m_blobNumber(0),
        m_currentBlobs(nullptr),
        m_blobminsize(0.001),
        m_blobmaxsize(0.01),
        m_threshold(10),
        m_infoOut(nullptr) {
    // initialize image
    m_image.xsize = 320;
    m_image.ysize = 240;
    m_image.setCsizeByFormat(GEM_GRAY);
    m_image.allocate();

    // outlets
    m_infoOut = outlet_new(this->x_obj, &s_list);

    m_blobNumber = static_cast<int>(f);
    if (m_blobNumber < 1) {
        m_blobNumber = 6;
    }
    numBlobsMess(m_blobNumber);

}

/*------------------------------------------------------------

Destructor

------------------------------------------------------------*/
pix_multiblob2::~pix_multiblob2() {
    outlet_free(m_infoOut);
    delete[]m_currentBlobs;
}

/*------------------------------------------------------------
makeBlobs
calculates the Blobs, maximal x and y values are set

algorithm adapted from imfill from animal.sf.net
by Ricardo Fabbri (labmacambira.sf.net)
------------------------------------------------------------*/
void pix_multiblob2::makeBlob(Blob *pb, int x_ini, int y_ini) {
    if (pb == nullptr) {
        return;
    }

    point *cp, np; // current pixel
    pstk_ptr current; // stack of current pixels

    point seed;
    seed.x = x_ini;
    seed.y = y_ini;
    current = new_pstk();
    ptpush(&current, &seed);
    do {
        cp = ptpop(&current);
        assert(cp);

        pb->area++;
        t_float grey = (static_cast<t_float>(m_image.GetPixel(cp->y, cp->x,
                                                              chGray)) / 255.0);
        double x = static_cast<t_float>(cp->x);
        double y = static_cast<t_float>(cp->y);
        pb->m_xaccum += grey * x;
        pb->m_yaccum += grey * y;
        pb->m_xyaccum += grey;
        pb->m_11 += grey * x * y;
        pb->m_20 += grey * x * x;
        pb->m_02 += grey * y * y;

        if (cp->x < pb->xmin()) {
            pb->xmin(cp->x);
        }
        if (cp->x > pb->xmax()) {
            pb->xmax(cp->x);
        }
        if (cp->y < pb->ymin()) {
            pb->ymin(cp->y);
        }
        if (cp->y > pb->ymax()) {
            pb->ymax(cp->y);
        }

        m_image.SetPixel(cp->y, cp->x, chGray, 0);
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                np.x = cp->x + j;
                np.y = cp->y + i;

                if (np.x >= 0 && np.y >= 0 &&
                    np.x < m_image.xsize && np.y < m_image.ysize &&
                    m_image.GetPixel(np.y, np.x, chGray) > m_threshold) {
                    ptpush(&current, &np);
                }
            }
        }
        free(cp);
    } while (pstk_isnt_empty(current));

    free_pstk(&current);
}

/*------------------------------------------------------------
addToBlobArray
adds a detected Blob to the blob list
------------------------------------------------------------*/
void pix_multiblob2::addToBlobArray(Blob *pb, int blobNumber) {
    if (blobNumber >= m_blobNumber) {
        // look whether we can replace a smaller blob
        float min = pb->area;
        int index = -1;
        int i = m_blobNumber;
        while (i--)
            if (m_currentBlobs[i].area < min) {
                min = m_currentBlobs[i].area;
                index = i;
            }
        if (index != -1) {
            m_currentBlobs[index] = *pb;
        }
    } else {
        m_currentBlobs[blobNumber] = *pb;
    }
}

/*------------------------------------------------------------

render

------------------------------------------------------------*/
void pix_multiblob2::doProcessing() {
    int blobminarea = static_cast<int>(m_blobminsize * m_image.xsize * m_image.ysize);
    int blobmaxarea = static_cast<int>(m_blobmaxsize * m_image.xsize * m_image.ysize);

    // reset the currentblobs array
    // detect blobs and add them to the currentBlobs-array
    int blobNumber = 0;
    for (int y = 0; y < m_image.ysize; y++) {
        for (int x = 0; x < m_image.xsize; x++) {
            if (m_image.GetPixel(y, x, 0) > 0) {
                Blob *blob = new Blob();
                blob->xmin(m_image.xsize);
                blob->ymin(m_image.ysize);

                makeBlob(blob, x, y);
                if (blob->area >= blobminarea && blob->area <= blobmaxarea) {
                    addToBlobArray(blob, blobNumber);
                    ++blobNumber;
                }
                delete blob;
            }
        }
    }

    // ok, we have found some blobs

    // since we can only handle m_blobNumber blobs, we might want to clip
    if (blobNumber > m_blobNumber) {
        blobNumber = m_blobNumber;
    }

    t_float scaleX = 1. / m_image.xsize;
    t_float scaleY = 1. / m_image.ysize;
    t_float scaleXY = scaleX * scaleY;

    // now create a matrix of [blobNumber*3] elements
    // each row holds all information on our blob
    auto *ap = new t_atom[2 + blobNumber * 9];
    SETFLOAT(ap, static_cast<t_float>(blobNumber));
    SETFLOAT(ap + 1, 9.0);

    for (int bn = 0; bn < blobNumber; bn++) {
        SETFLOAT(ap + bn * 9 + 2, m_currentBlobs[bn].xmid() * scaleX); // weighted X
        SETFLOAT(ap + bn * 9 + 3, m_currentBlobs[bn].ymid() * scaleY); // weighted Y
        SETFLOAT(ap + bn * 9 + 4, m_currentBlobs[bn].m_xyaccum * scaleXY); // weighted Area

        SETFLOAT(ap + bn * 9 + 5, m_currentBlobs[bn].xmin() * scaleX); // minX
        SETFLOAT(ap + bn * 9 + 6, m_currentBlobs[bn].ymin() * scaleY); // minY
        SETFLOAT(ap + bn * 9 + 7, m_currentBlobs[bn].xmax() * scaleX); // maxX
        SETFLOAT(ap + bn * 9 + 8, m_currentBlobs[bn].ymax() * scaleY); // maxY

        SETFLOAT(ap + bn * 9 + 9, m_currentBlobs[bn].area * scaleXY);  // unweighted Area
        SETFLOAT(ap + bn * 9 + 10, m_currentBlobs[bn].angle());      // weighted orientation
    }

    // i admit that it is naughty to use "matrix" from zexy/iemmatrix
    // but it is the best thing i can think of for 2-dimensional arrays
    outlet_anything(m_infoOut, gensym("matrix"), 2 + 9 * blobNumber, ap);

    delete[]ap;
}

void pix_multiblob2::processImage(imageStruct &image) {
//     store the image in greyscale
//     since the algorithm is destructive we do it in a sandbox...
    m_image.convertFrom(&image);
    doProcessing();
}


/*------------------------------------------------------------
blobMinSizeMess
------------------------------------------------------------*/
void pix_multiblob2::blobMinSizeMess(t_float blobMinSize) {
    if ((blobMinSize < 0.0) || (blobMinSize > 1.0)) {
        error("blobMinSize %f out of range (0..1)!", blobMinSize);
        return;
    }
    m_blobminsize = blobMinSize / 100.0;
    post("blobminsize:%f", m_blobminsize);
    blobSizeWarning();
}

/*------------------------------------------------------------
blobMaxSizeMess
------------------------------------------------------------*/
void pix_multiblob2::blobMaxSizeMess(t_float blobMaxSize) {
    if ((blobMaxSize < 0.0) || (blobMaxSize > 100.0)) {
        error("blobMaxSize %f out of range (0..100)!", blobMaxSize);
        return;
    }
    m_blobmaxsize = blobMaxSize / 100.0;
    post("blobmaxsize:%f", m_blobmaxsize);
    blobSizeWarning();
}

/*------------------------------------------------------------
blobSizeWarning
------------------------------------------------------------*/
void pix_multiblob2::blobSizeWarning() const {
    if (m_blobminsize > m_blobmaxsize) {
        post("[warning] pix_multiblob2: minimum blob size greater than maximum blob size");
    }
}

/*------------------------------------------------------------
threshMess
------------------------------------------------------------*/
void pix_multiblob2::threshMess(t_float thresh) {
    if ((thresh < 0.0) || (thresh > 1.0)) {
        error("threshold %f out of range (0..1)!", thresh);
    }
    m_threshold = CLAMP(thresh * 255);
}

/*------------------------------------------------------------
threshMess
------------------------------------------------------------*/
void pix_multiblob2::numBlobsMess(unsigned int blobs) {
    delete[]m_currentBlobs;

    // initialize blob-structures
    m_currentBlobs = new Blob[blobs];
    m_blobNumber = blobs;
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_multiblob2::obj_setupCallback(t_class *classPtr) {
    CPPEXTERN_MSG1(classPtr, "blobSize", blobMinSizeMess, t_float);
    CPPEXTERN_MSG1(classPtr, "blobMinSize", blobMinSizeMess, t_float);
    CPPEXTERN_MSG1(classPtr, "blobMaxSize", blobMaxSizeMess, t_float);
    CPPEXTERN_MSG1(classPtr, "thresh", threshMess, t_float);
    CPPEXTERN_MSG1(classPtr, "threshold", threshMess, t_float);

    CPPEXTERN_MSG1(classPtr, "blobs", numBlobsMess, unsigned int);
}
