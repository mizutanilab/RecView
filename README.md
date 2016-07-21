## RecView
RecView is a program for microtomographic reconstruction and image processing. It consists of about 14,000 lines of custom source codes in C++ and CUDA C and x86/x64 machine languages. RecView is designed for processing tomographic data obtained at the BL20B2, BL20XU, BL37XU, and BL47XU beamlines of the synchrotron radiation facility SPring-8 and also those taken at the 32-ID-BC station of Advanced Photon Source, Argonne National Laboratory.

<IMG width=100 height=140 alt=testPattern src="pics/testPattern.png" align=left>
Three-dimensional structural analysis by synchrotron radiation microtomography is performed by recording two-dimensional radiographs. Then the three-dimensional structure is determined by the convolution back-projection method. This reconstruction calculation is repeated for each tomographic slice, giving the three-dimensional structure. Therefore, the resolution in the tomographic slice should be affected by the reconstruction calculation. The resolutions in the tomographic slice plane and perpendicular to it have been examined from the modulation transfer functions. The through-plane resolution perpendicular to the tomographic slice corresponded to the spatial resolution of two-dimensional radiographs. By contrast, the in-plane resolution within the slice was approximately 1.5-times as large as the through-plane resolution. Although the two-dimensional radiographs were taken with the pixel width of half the x-ray optics resolution, the three-dimensional resolution analyses indicated that zoom reconstruction should be performed to achieve in-plane resolution comparable to the x-ray optics resolution. <i>Micron</i> <b>41(1)</b>, 90-95. <a href="https://drive.google.com/open?id=0Byx6vGOSewwpckZhYTAxZzVaLVk">preprint</a>
<BR clear=left>

## License
RecView is provided under the <a href="https://sites.google.com/site/mizutanilab1/software-recview/license">BSD 2-Clause License</a>.

## References
<ul><li>R. Mizutani, A. Takeuchi, R.Y. Osamura, S. Takekoshi, K. Uesugi and Y. Suzuki (2010). Submicrometer tomographic resolution examined using a micro-fabricated test object. <i>Micron</i> <b>41(1)</b>, 90-95. 
<a href="http://dx.doi.org/10.1016/j.micron.2009.09.001">DOI</a>
<a href="http://www.ncbi.nlm.nih.gov/pubmed/19800246">PubMed</a>
<a href="https://drive.google.com/open?id=0Byx6vGOSewwpckZhYTAxZzVaLVk">preprint</a>
</li>
<li>R. Mizutani, K. Taguchi, A. Takeuchi, K. Uesugi and Y. Suzuki (2010). Estimation of presampling modulation transfer function in synchrotron radiation microtomography. <i>Nuclear Instrum. Meth. A</i> <b>621(1-3)</b>, 615-619.
<a href="http://dx.doi.org/10.1016/j.nima.2010.03.111">DOI</a>
<a href="https://drive.google.com/open?id=0Byx6vGOSewwpUk9WRWpJdmVDajg">preprint</a>
</li>
<li>R. Mizutani, A. Takeuchi, K. Uesugi, S. Takekoshi, N. Nakamura and Y. Suzuki (2011). Building human brain network in 3D coefficient map determined by X-ray microtomography. <i>AIP Conference Proceedings</i> <b>1365</b>, 403-406.
<a href="http://link.aip.org/link/?APCPCS/1365/403/1">Abstract and full-text pdf</a>
<a href="https://drive.google.com/open?id=0Byx6vGOSewwpbXJ2TzV4WUROOGc">preprint</a> (right-click to download)
</li>

<li>R. Mizutani and Y. Suzuki (2012). X-ray Microtomography in biology. <i>Micron</i> <b>43(2-3)</b>, 104-115. Review. 
<a href="http://dx.doi.org/10.1016/j.micron.2011.10.002">DOI</a>
<a href="http://www.ncbi.nlm.nih.gov/pubmed/22036251">PubMed</a>
<a href="https://drive.google.com/open?id=0Byx6vGOSewwpU2JXZy1xMkRzMkU">preprint</a>
</li>

<li>R. Mizutani, R. Saiga, S. Takekoshi, C. Inomoto, N. Nakamura, M. Itokawa, M. Arai, K. Oshima, A. Takeuchi, K. Uesugi, Y. Terada and Y. Suzuki (2016). A method for estimating spatial resolution of real image in the Fourier domain. <i>J. Microscopy</i> <b>261(1)</b>, 57-66.
<a href="http://dx.doi.org/10.1111/jmi.12315">DOI</a>
<a href="http://www.ncbi.nlm.nih.gov/pubmed/26444300">PubMed</a>
<a href="https://drive.google.com/open?id=0Byx6vGOSewwpR2lJOHlQaGtkWms">preprint</a>
</li>
</ul>

## Release notes
The binary folder contains several executables. Please use the 'RecView' executable appropriate for your PC platform. If your PC has NVIDIA Tesla, GeForce or Quadro GPU processors, you can use the CUDA version. The dynamic link libraries (64 bit: 'cudart64_55.dll' and 'cufft64_55.dll'; 32 bit: 'cudart32_55.dll' and 'cufft32_55.dll') should be placed in the folder of RecView CUDA executable. These library files are also available from the <a href="http://www.nvidia.com/object/cuda_home.html">official NVIDIA site</a> as part of the CUDA toolkit. ATI stream processors Radeon, FireStream or FirePro are also supported. To use ATI processors, you should install the OpenCL library 'OpenCL.dll' provided as part of the <a href="http://game.amd.com/us-en/drivers_catalyst.aspx">ATI Catalyst suite</a>. Source codes are not available at present.

A test dataset in TIFF format is also provided in the binary folder.

<UL>
  <LI>v4.9.0 (released 21 Jul 2016). APS data in HDF5 format is now supported.
  <LI>v4.7.0 (released 13 Nov 2015). A dedicated routine for resolution estimation plot (J. Microsc. 2015) was implemented. CSV files now can be generated from 'Analysis==>Resolution plot' menu. ATI processors are not supported in this release.
  <LI>v4.5.0 (released 6 Jan 2015). Update to support recent output.log format. Fourier transformations now can be generated from Tomography menu. A faster HIS-file reading routine was also implemented. A problem with the tilt angle direction of x64 reconstruction routine was fixed. User interfaces were updated.
  <LI>v4.0.2 (released 22 Oct 2013). Back projection routines running on CUDA processors were updated to support the CUDA 5.5 computing environment. The CUDA version for x64 platforms was also included from this release. Queues from multiple RecView instances are now executed sequentially. 
  <LI>v3.5.1 (released 16 Feb 2013). Functions for rotation center determination were revised. 
  <LI>v3.4.4 (released 1 Oct 2012). Drift correction functions were revised. Image reconstruction and processing are now logged in plain text format in the dataset folder. Queueing functions were updated to support least-square fitting of 3D images.
  <LI>v3.1.0 (released 23 Sep 2011). HIS (Hamamatsu Image Sequence) format is now supported. Source codes for reading HIS format files were kindly provided by Dr. Uesugi, JASRI/SPring-8. Reconstruction routines were revised to support images taken with Zernike phase contrast.
  <LI>v3.0.1 (released 12 Jan 2011). Least-square fitting functions were provided. Reconstruction routines were revised to support raw image files in the TIFF format.
  <LI>v3.0.0 (released 27 May 2010). Back projection routines running on Tesla, GeForce, Quadro processors were updated to support the CUDA 3.0 computing environment. The OpenCL 1.0 routines for ATI Radeon, FireStream or FirePro processors were also implemented. Performances are:
    <UL>
    <LI>Tau (448 cores, 607 MHz) = 0.0813 nsec (23.4 sec for a 8000x8000 tomogram from 4500 projections) by using GeForce GTX470 with 1.28 GB of GDDR5 memory,
    <LI>Tau (112 cores, 500 MHz) = 0.243 nsec (1.75 sec for a 2000x2000 tomogram from 1800 projections) by using Quadro FX 3700 with 512 MB of GDDR3 memory,
    <LI>Tau (80 cores, 650 MHz) = 2.86 nsec (20.6 sec for a 2000x2000 tomogram from 1800 projections) by using Radeon HD 5450 with 512 MB of DDR2 memory,
    <LI>Tau (4 threads, 3.16 GHz) = 0.825 nsec (5.94 sec for a 2000x2000 tomogram from 1800 projections) by using Xeon (x86),
    <LI>Tau (4 threads, 3.16 GHz) = 0.808 nsec (5.82 sec for a 2000x2000 tomogram from 1800 projections) by using Xeon (x64).
    </UL>
Here, tau is the time constant required for one pixel projection. For example, one tomogram of 2000 x 2000 pixels from 1800 projection frames can be reconstructed in tau x 2000 x 2000 x 1800 sec. Drift correction options were also provided in this revision.
  <LI>v2.0.10 (released 2 Sep 2009). Tilted reconstruction was supported. Angular interpolation was implemented for compatibility with the ct_cbp reconstruction suite. Trimming of marginal regions can be performed in the 8-bit TIFF conversion process.
  <LI>v2.0.6 (released 13 Mar 2009). Bug-fix release revising memory allocation procedures in the CUDA reconstruction routine.
  <LI>v2.0.5 (released 9 Feb 2009). Back-projection routines running on Tesla, GeForce or Quadro processors supporting the CUDA programming environment were implemented. The x86 and x64 assembler routines were also revised. Performances are:
    <UL>
    <LI>Tau (112 cores, 500 MHz) = 0.314 nsec (2.28 sec for a 2000x2000 tomogram from 1800 projections) using a Quadro FX 3700,
    <LI>Tau (3.16 GHz) = 0.633 nsec (4.56 sec for a 2000x2000 tomogram from 1800 projections) using Core2Duo (x86, 2 threads), or tau = 1.336 nsec (x86, single thread),
    <LI>Tau (3.16 GHz) = 0.802 nsec (5.77 sec for a 2000x2000 tomogram from 1800 projections) by using Xeon (x86, 4 threads), or tau = 1.724 nsec (x86, single thread),
    <LI>Tau (3.16 GHz) = 0.768 nsec (5.53 sec for a 2000x2000 tomogram from 1800 projections) by using Xeon (x64, 4 threads), or tau = 1.651 nsec (x64, single thread).
    </UL>
  <LI>v2.0.0 (released 21 Jan 2009). x64 platform is now supported. The x64 version can generate larger tomograms upto 10<sup>6</sup> x 10<sup>6</sup> pixels. Performance: tau(3.16 GHz) = 0.789 nsec using Xeon (x64, 4 threads). Performace of x86 version: tau(3.16 GHz) = 0.849 nsec using Core2Duo (x86, 2 threads), or tau = 1.395 nsec (x86, single thread).
  <LI>v1.0.7 (released 23 Aug 2008). Interpolated tomographic reconstruction was implemented. Performance: tau(3.16 GHz) = 1.209 nsec using Core2Duo (x86, 2 threads).
  <LI>v1.0.4 (released 1 Apr 2008). Multithreaded reconstruction routine was implemented. This function is partially written in x86 machine language, allowing faster execution of the reconstruction calculation.
  <LI>v1.0.0 (released 6 Mar 2008).
</LI></UL>

## How to use
Concise help can be found in the 'Help'-'About' menu. We believe that this program is self-explanatory, but the following tips should be helpul.

<B>Installation</B>  
Download the RecView compressed file. Extract the executable and corresponding library files in any folder you like. 

<B>Execution</B>  
Double click the 'RecView' executable.

<B>Open radiograph</B>  
From the menu bar, select 'File'-'Open...' and choose an ITEX file. Click the 'OK' button. 

<B>Computing environment</B>  
The GPU or CPU computing environment can be selected in the 'Tomography'-'Property' dialog.

<B>Reconstruction</B>  
Open 'Tomography'-'Reconstruction...' dialog. Enter appropriate parameters for your reconstruction calculation. 'Get center' determines the position of the rotation axis. After you have examined a tomogram by using 'Show image', 'Batch' executes the reconstruction calculations.

<B>Trimming or reformatting images</B>  
The 'Tomography'-'Histogram=>8bitTIF...' menu provides several tools for generating 8-bit TIFF images.

## Frequently asked questions
<OL>
  <LI><b>System requirements</b></LI>
    RecView can be executed on a Windows PC running XP, Vista, or Windows 7 with an x86 or x64 CPU and local storage.<br><br>
  <LI><b>Manuals</b></LI>
    A brief how-to-use guide has been published as the appendix of the following paper:<BR><BR>
R. Mizutani, A. Takeuchi, K. Uesugi, S. Takekoshi, R.Y. Osamura and Y. Suzuki (2009). Three-dimensional microstructural analysis of human brain tissue by using synchrotron radiation microtomographs. In <I>Handbook on White Matter</I>, eds. Westland, T.B. & Calton, R.N., New York, Nova Science Publishers, pp. 247-277.
<A href="http://www.el.u-tokai.ac.jp/ryuta/WhiteMatterChap14Mizutani.pdf">pdf</A> (9.5 MB)<br><br>
  <LI><b>What kind of data can be processed?</b></LI>
    RecView is designed for the reconstruction of tomographic data obtained at SPring-8 and APS. However, any kind of data can be processed. Please contact the author if you have problems in using this program with data from other tomographs. Dataset requirements are:
    <UL>
      <LI>We use file names beginning with alphabet characters followed by frame number, such as 'q0005.tif'.</LI>
      <LI>Place a dark-field image 'dark.tif' in the dataset folder.</LI>
      <LI>RecView also needs a paprameter file 'output.log' in plain text format (an example is given in the test dataset). This file has four fields:<BR>
<pre>
frame#      time(msec)     angle(deg) 0=flatfield/1=sample
00001       00012.46900    000.0000        0
00002       00019.26600    000.1000        1
00003       00021.34400    000.2000        1
00004       00023.43800    000.3000        1
00005       00025.51600    000.4000        1
...
01890       05298.43800    180.0000        1
01891       05310.65700    180.0000        0</pre>
          Spreadsheet softwares work well for manually generating this 'output.log' file. The 'time' fields are used for interpolating the trend of flatfield pixel intensities. <BR>The format of output.log files of recent SPring-8 sessions was revised to list angle in pulses instead of angle in degrees. The latest executable can recognize each format.
      </LI>
    </UL><br>
  <LI><b>How to try the test dataset?</b></LI>
    <UL>
      <LI>Extract all files in the archived test dataset.</LI>
      <LI>Open one of TIFF files (such as Q0005.tif) using RecView.</LI>
      <LI>Open 'Tomography'-'Reconstruction...' dialog.</LI>
      <LI>Enter '380' in the 'From' layer field.</LI>
      <LI>Enter '311' in the 'From' rotation-center field.</LI>
      <LI>Click 'Show image' below. In seconds, a cross section of a square-wave pattern carved on an aluminum wire will be shown.</LI>
    </UL><br>
  <LI><b>Does RecView make network connections?</b></LI>
      No. However, remote folders such as workgroup PCs are searched when you open files. This is  performed by the Windows dialog function.
</OL>

## Contact
Ryuta Mizutani, Dr., Prof.  
Department of Applied Biochemistry  
School of Engineering, Tokai University  
Kitakaname 4-1-1, Hiratsuka, Kanagawa 259-1292, Japan  
E-mail ryuta(at)tokai-u.jp  
HP https://sites.google.com/site/mizutanilab1/  
<A href="http://www.linkedin.com/pub/ryuta-mizutani/79/832/115">Linkedin</A> - 
<A href="http://www.facebook.com/people/Ryuta-Mizutani/100005433369640">Facebook</A><BR>

