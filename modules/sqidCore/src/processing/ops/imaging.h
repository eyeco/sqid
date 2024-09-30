/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <processing/op.h>

namespace sqid
{
	namespace Imaging
	{
		class LowPass : public Op
		{
		public:
			enum Kernel
			{
				K_GAUSS,

				K_COUNT
			};

			static const char *kernelToString( Kernel k );
			static Kernel kernelFromString( const char *s );
			static Kernel kernelFromString( const std::string &s );

		private:
			int _size;
			float _sigma;
			bool _autoSigma;

			Kernel _kernel;

			cv::Mat _k;

			void updateKernel();

		protected:
			virtual bool process();

		public:
			LowPass( Kernel kernel = K_GAUSS );
			virtual ~LowPass();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class HighPass : public Op
		{
		public:
			enum Kernel
			{
				K_SOBEL,
				K_SCHARR,
				//TODO: add laplacian via cv::Laplacian https://docs.opencv.org/2.4/modules/imgproc/doc/filtering.html#laplacian

				K_COUNT
			};

			static const char *kernelToString( Kernel k );
			static Kernel kernelFromString( const char *s );
			static Kernel kernelFromString( const std::string &s );

		private:
			bool _horizontally;

			int _deriv;
			int _aperture;

			bool _normalize;

			Kernel _kernel;

			cv::Mat _kHor;
			cv::Mat _kVer;

			void updateKernel();

		protected:
			virtual bool process();

		public:
			HighPass( Kernel kernel = K_SOBEL );
			virtual ~HighPass();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Blur : public Op
		{
		public:
			enum Method
			{
				M_BOX,
				M_GAUSS,
				M_MEDIAN,

				M_COUNT
			};

			static const char *methodToString( Method dm );
			static Method methodFromString( const char *s );
			static Method methodFromString( const std::string &s );

		private:
			Method _method;

			unsigned int _size;
			float _sigma;

		protected:
			virtual bool process();

		public:
			Blur();
			virtual ~Blur();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Morph : public Op
		{
		public:
			enum Type
			{
				T_ERODE,
				T_DILATE,
				T_OPEN,
				T_CLOSE,
				//TODO: add gradiend, top hat, black hat, hit miss

				T_COUNT
			};

			static const char *typeToString( Type t );
			static Type typeFromString( const char *s );
			static Type typeFromString( const std::string &s );

		private:
			Type _type;

			int _iterations;

			cv::Mat _kernel;

			void updateKernel();

		protected:
			virtual bool process();

		public:
			Morph( Type type = T_CLOSE );
			virtual ~Morph();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};


		/*
		//TODO: implement
		class CameraIntrinsics : public FrameProcessor
		{
		private:

		protected:
			virtual bool process();

		public:
			CameraIntrinsics();
			virtual ~CameraIntrinsics();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO: implement
		class CameraExtrinsics : public FrameProcessor
		{
		private:

		protected:
			virtual bool process();

		public:
			CameraExtrinsics();
			virtual ~CameraExtrinsics();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
		*/
	}
}
