/*****************************************************************************************
 * File: RenderText2dDirectX.cpp
 * Desc: Bitmap fonts text blitting
 *****************************************************************************************/

/*
IndieLib 2d library Copyright (C) 2005 Javier L�pez L�pez (info@pixelartgames.com)
MODIFIED BY Miguel Angel Qui�ones (2011) (mail:m.quinones.garcia@gmail.com / mikeskywalker007@gmail.com)

This library is free software; you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
*/
#include "Defines.h"

#ifdef INDIERENDER_DIRECTX

/** @cond DOCUMENT_PRIVATEAPI */

// ----- Includes -----

#include "Global.h"
#include "IND_SurfaceManager.h"
#include "IND_Surface.h"
#include "IND_Font.h"
#include "DirectXRender.h"


// --------------------------------------------------------------------------------
//							         Public methods
// --------------------------------------------------------------------------------

void DirectXRender::blitText(IND_Font *pFo,
                             char *pText,
                             int pX,
                             int pY,
                             int pOffset,
                             int pLineSpacing,
                             float pScaleX,
                             float pScaleY,
                             unsigned char pR,
                             unsigned char pG,
                             unsigned char pB,
                             unsigned char pA,
                             unsigned char pFadeR,
                             unsigned char pFadeG,
                             unsigned char pFadeB,
                             unsigned char pFadeA,
                             IND_Filter pLinearFilter,
                             IND_BlendingType pSo,
                             IND_BlendingType pDs,
                             IND_Align pAlign) {
	// ----- Transform -----

	bool correctParams = true;
	if(!pFo->getSurface()) {
		correctParams = false;
	}

	if (correctParams) {
		//SetTransform2d (pX, pY, 0, 0, 0, 1, 1, 0, 0, 0, 0, pFo->getSurface()->getWidthBlock(), pFo->getSurface()->getHeightBlock(), 0);
		setTransform2d(pX, pY, 0, 0, 0, pScaleX, pScaleY, 0, 0, 0, 0, pFo->getSurface()->getWidthBlock(), pFo->getSurface()->getHeightBlock(), 0);
		setRainbow2d(pFo->_font._surface->getTypeInt(), 1, 0, 0, pLinearFilter, pR, pG, pB, pA, pFadeR, pFadeG, pFadeB, pFadeA, pSo, pDs);

		// ----- Drawing text   -----

		unsigned char mChar1;
		unsigned char mChar2;
		int mCont1 = 0;
		int mCont2 = 0;
		int mTranslationX = 0;
		int mTranslationY = 0;
		bool mErrorChar;    // Char that doesn't exist
		int mSentencePos;
		int mLongActualSentence;

		D3DXMATRIX mMatTraslation, mMatWorld, mMatWorldOriginal;
		_info._device->GetTransform(D3DTS_WORLD, &mMatWorld);
		_info._device->GetTransform(D3DTS_WORLD, &mMatWorldOriginal);

		mCont1 = 0;
		mChar1 = pText [mCont1++];

		while (mChar1 != 0) {
			// If it's a new line or it's the first line
			if (mChar1 == '\n' || mCont1 == 1) {
				// First line
				if (mCont1 == 1)
					mSentencePos = 0;
				else
					mSentencePos = mCont1;

				// Set the alignment
				switch (pAlign) {
				case IND_CENTER: {
					//mLongActualSentence = GetLongInPixels (pFo, pText, mSentencePos, pOffset);
					mLongActualSentence = static_cast<int>(getLongInPixels(pFo, pText, mSentencePos, pOffset) * pScaleX);
					mTranslationX = (int)(mLongActualSentence / 2);
					break;
				}

				case IND_RIGHT: {
					//mLongActualSentence = GetLongInPixels (pFo, pText, mSentencePos, pOffset);
					mLongActualSentence = static_cast<int>(getLongInPixels(pFo, pText, mSentencePos, pOffset) * pScaleX);
					mTranslationX = (int)(mLongActualSentence);

					break;
				}

				case IND_LEFT: {
					break;
				}
				}

				D3DXMatrixTranslation(&mMatTraslation, (float) - mTranslationX, (float) mTranslationY, 0);
				//mTranslationY += (pLineSpacing);
				mTranslationY += static_cast<int>((pLineSpacing * pScaleY));
				D3DXMatrixMultiply(&mMatWorld, &mMatWorldOriginal, &mMatTraslation);
				_info._device->SetTransform(D3DTS_WORLD, &mMatWorld);
			}

			// It's a normal character
			if (mChar1 != '\n') {

				mErrorChar = 0;
				mCont2 = 0;
				mChar2 = pFo->getLetters() [mCont2++]._letter;

				// Seek its location in the bitmap
				while (mChar2 != mChar1 && mCont2 < pFo->getNumChars()) mChar2 = pFo->getLetters() [mCont2++]._letter;
				if (mCont2  ==  pFo->getNumChars())
					mErrorChar = 1;

				mCont2--;
				if (!mErrorChar) {
					blitRegionSurface(pFo->getSurface(),
									  pFo->getLetters() [mCont2]._offsetX + 1,
									  pFo->getLetters() [mCont2]._offsetY + 1,
									  pFo->getLetters() [mCont2]._widthChar - 1,
									  pFo->getLetters() [mCont2]._heightChar - 1);
				}


				// Displacement of the character
				//SetTranslation ((pFo->getLetters() [mCont2]._widthChar) + pOffset, 0, &mMatWorld, &mMatTraslation);
				SetTranslation(static_cast<int>(((pFo->getLetters() [mCont2]._widthChar) + pOffset) * pScaleX), 0, &mMatWorld, &mMatTraslation);
			}

			// Advance one character
			mChar1 = pText [mCont1++];
		}
	}
}

// --------------------------------------------------------------------------------
//							         Private methods
// --------------------------------------------------------------------------------

/*
==================
Returns a long in pixels of the sentence that starts in pPos
A sentence is an array of chars that ends in \n or \0
==================
*/
int DirectXRender::getLongInPixels(IND_Font *pFo, char *pText, int pPos, int pOffset) {
	unsigned char mChar1;
	unsigned char mChar2;
	int mCont1 = pPos;
	int mCont2 = 0;
	bool mErrorChar;    // Error when nonexistant char

	int mWidthSentence = 0;
	mChar1 = pText [mCont1++];
	while (mChar1 != '\0' && mChar1 != '\n') {
		mErrorChar = 0;
		mCont2 = 0;
		mChar2 = pFo->getLetters() [mCont2++]._letter;

		// Search the position
		while (mChar2 != mChar1 && mCont2 < pFo->getNumChars()) mChar2 = pFo->getLetters() [mCont2++]._letter;
		if (mCont2  ==  pFo->getNumChars())
			mErrorChar = 1;

		if (!mErrorChar)
			mWidthSentence += pFo->getLetters() [mCont2 - 1]._widthChar + pOffset;

		mChar1 = pText [mCont1++];
	}

	return mWidthSentence;
}


/*
==================
Set translation (used for translating the chars)
==================
*/
void DirectXRender::SetTranslation(int pX, int pY, D3DXMATRIX *pMatWorld, D3DXMATRIX *pMatTraslation) {
	D3DXMatrixTranslation(pMatTraslation, (float) pX, (float) pY, 0);
	D3DXMatrixMultiply(pMatWorld, pMatWorld, pMatTraslation);
	_info._device->SetTransform(D3DTS_WORLD, pMatWorld);
}

/** @endcond */

#endif //INDIERENDER_DIRECTX
