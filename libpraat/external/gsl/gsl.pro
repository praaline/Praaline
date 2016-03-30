! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = gsl

INCLUDEPATH += ../../sys ../../dwsys

SOURCES = \
   gsl_blas__blas.c gsl_block__block.c gsl_block__file.c \
   gsl_block__init.c gsl_bspline__bspline.c gsl_cblas__caxpy.c \
   gsl_cblas__ccopy.c gsl_cblas__cdotc_sub.c gsl_cblas__cdotu_sub.c \
   gsl_cblas__cgbmv.c gsl_cblas__cgemm.c gsl_cblas__cgemv.c \
   gsl_cblas__cgerc.c gsl_cblas__cgeru.c gsl_cblas__chbmv.c \
   gsl_cblas__chemm.c gsl_cblas__chemv.c gsl_cblas__cher.c \
   gsl_cblas__cher2.c gsl_cblas__cher2k.c gsl_cblas__cherk.c \
   gsl_cblas__chpmv.c gsl_cblas__chpr.c gsl_cblas__chpr2.c \
   gsl_cblas__cscal.c gsl_cblas__csscal.c gsl_cblas__cswap.c \
   gsl_cblas__csymm.c gsl_cblas__csyr2k.c gsl_cblas__csyrk.c \
   gsl_cblas__ctbmv.c gsl_cblas__ctbsv.c gsl_cblas__ctpmv.c \
   gsl_cblas__ctpsv.c gsl_cblas__ctrmm.c gsl_cblas__ctrmv.c \
   gsl_cblas__ctrsm.c gsl_cblas__ctrsv.c gsl_cblas__dasum.c \
   gsl_cblas__daxpy.c gsl_cblas__dcopy.c gsl_cblas__ddot.c \
   gsl_cblas__dgbmv.c gsl_cblas__dgemm.c gsl_cblas__dgemv.c \
   gsl_cblas__dger.c gsl_cblas__dnrm2.c gsl_cblas__drot.c \
   gsl_cblas__drotg.c gsl_cblas__drotm.c gsl_cblas__drotmg.c \
   gsl_cblas__dsbmv.c gsl_cblas__dscal.c gsl_cblas__dsdot.c \
   gsl_cblas__dspmv.c gsl_cblas__dspr.c gsl_cblas__dspr2.c \
   gsl_cblas__dswap.c gsl_cblas__dsymm.c gsl_cblas__dsymv.c \
   gsl_cblas__dsyr.c gsl_cblas__dsyr2.c gsl_cblas__dsyr2k.c \
   gsl_cblas__dsyrk.c gsl_cblas__dtbmv.c gsl_cblas__dtbsv.c \
   gsl_cblas__dtpmv.c gsl_cblas__dtpsv.c gsl_cblas__dtrmm.c \
   gsl_cblas__dtrmv.c gsl_cblas__dtrsm.c gsl_cblas__dtrsv.c \
   gsl_cblas__dzasum.c gsl_cblas__dznrm2.c gsl_cblas__icamax.c \
   gsl_cblas__idamax.c gsl_cblas__isamax.c gsl_cblas__izamax.c \
   gsl_cblas__sasum.c gsl_cblas__saxpy.c gsl_cblas__scasum.c \
   gsl_cblas__scnrm2.c gsl_cblas__scopy.c gsl_cblas__sdot.c \
   gsl_cblas__sdsdot.c gsl_cblas__sgbmv.c gsl_cblas__sgemm.c \
   gsl_cblas__sgemv.c gsl_cblas__sger.c gsl_cblas__snrm2.c \
   gsl_cblas__srot.c gsl_cblas__srotg.c gsl_cblas__srotm.c \
   gsl_cblas__srotmg.c gsl_cblas__ssbmv.c gsl_cblas__sscal.c \
   gsl_cblas__sspmv.c gsl_cblas__sspr.c gsl_cblas__sspr2.c \
   gsl_cblas__sswap.c gsl_cblas__ssymm.c gsl_cblas__ssymv.c \
   gsl_cblas__ssyr.c gsl_cblas__ssyr2.c gsl_cblas__ssyr2k.c \
   gsl_cblas__ssyrk.c gsl_cblas__stbmv.c gsl_cblas__stbsv.c \
   gsl_cblas__stpmv.c gsl_cblas__stpsv.c gsl_cblas__strmm.c \
   gsl_cblas__strmv.c gsl_cblas__strsm.c gsl_cblas__strsv.c \
   gsl_cblas__xerbla.c gsl_cblas__zaxpy.c gsl_cblas__zcopy.c \
   gsl_cblas__zdotc_sub.c gsl_cblas__zdotu_sub.c gsl_cblas__zdscal.c \
   gsl_cblas__zgbmv.c gsl_cblas__zgemm.c gsl_cblas__zgemv.c \
   gsl_cblas__zgerc.c gsl_cblas__zgeru.c gsl_cblas__zhbmv.c \
   gsl_cblas__zhemm.c gsl_cblas__zhemv.c gsl_cblas__zher.c \
   gsl_cblas__zher2.c gsl_cblas__zher2k.c gsl_cblas__zherk.c \
   gsl_cblas__zhpmv.c gsl_cblas__zhpr.c gsl_cblas__zhpr2.c \
   gsl_cblas__zscal.c gsl_cblas__zswap.c gsl_cblas__zsymm.c \
   gsl_cblas__zsyr2k.c gsl_cblas__zsyrk.c gsl_cblas__ztbmv.c \
   gsl_cblas__ztbsv.c gsl_cblas__ztpmv.c gsl_cblas__ztpsv.c \
   gsl_cblas__ztrmm.c gsl_cblas__ztrmv.c gsl_cblas__ztrsm.c \
   gsl_cblas__ztrsv.c gsl_cdf__beta.c gsl_cdf__betainv.c \
   gsl_cdf__binomial.c gsl_cdf__cauchy.c gsl_cdf__cauchyinv.c \
   gsl_cdf__chisq.c gsl_cdf__chisqinv.c gsl_cdf__exponential.c \
   gsl_cdf__exponentialinv.c gsl_cdf__exppow.c gsl_cdf__fdist.c \
   gsl_cdf__fdistinv.c gsl_cdf__flat.c gsl_cdf__flatinv.c \
   gsl_cdf__gamma.c gsl_cdf__gammainv.c gsl_cdf__gauss.c \
   gsl_cdf__gaussinv.c gsl_cdf__geometric.c gsl_cdf__gumbel1.c \
   gsl_cdf__gumbel1inv.c gsl_cdf__gumbel2.c gsl_cdf__gumbel2inv.c \
   gsl_cdf__hypergeometric.c gsl_cdf__laplace.c gsl_cdf__laplaceinv.c \
   gsl_cdf__logistic.c gsl_cdf__logisticinv.c gsl_cdf__lognormal.c \
   gsl_cdf__lognormalinv.c gsl_cdf__nbinomial.c gsl_cdf__pareto.c \
   gsl_cdf__paretoinv.c gsl_cdf__pascal.c gsl_cdf__poisson.c \
   gsl_cdf__rayleigh.c gsl_cdf__rayleighinv.c gsl_cdf__tdist.c \
   gsl_cdf__tdistinv.c gsl_cdf__weibull.c gsl_cdf__weibullinv.c \
   gsl_combination__combination.c gsl_combination__file.c gsl_combination__init.c \
   gsl_complex__math.c gsl_deriv__deriv.c gsl_dht__dht.c \
   gsl_diff__diff.c gsl_eigen__francis.c gsl_eigen__gen.c \
   gsl_eigen__genherm.c gsl_eigen__genhermv.c gsl_eigen__gensymm.c \
   gsl_eigen__gensymmv.c gsl_eigen__genv.c gsl_eigen__herm.c \
   gsl_eigen__hermv.c gsl_eigen__jacobi.c gsl_eigen__nonsymm.c \
   gsl_eigen__nonsymmv.c gsl_eigen__schur.c gsl_eigen__sort.c \
   gsl_eigen__symm.c gsl_eigen__symmv.c gsl_err__error.c \
   gsl_err__message.c gsl_err__stream.c gsl_err__strerror.c \
   gsl_fft__dft.c gsl_fft__fft.c gsl_fft__signals.c \
   gsl_fit__linear.c gsl_histogram__add.c gsl_histogram__add2d.c \
   gsl_histogram__calloc_range.c gsl_histogram__calloc_range2d.c gsl_histogram__copy.c \
   gsl_histogram__copy2d.c gsl_histogram__file.c gsl_histogram__file2d.c \
   gsl_histogram__get.c gsl_histogram__get2d.c gsl_histogram__init.c \
   gsl_histogram__init2d.c gsl_histogram__maxval.c gsl_histogram__maxval2d.c \
   gsl_histogram__oper.c gsl_histogram__oper2d.c gsl_histogram__params.c \
   gsl_histogram__params2d.c gsl_histogram__pdf.c gsl_histogram__pdf2d.c \
   gsl_histogram__reset.c gsl_histogram__reset2d.c gsl_histogram__stat.c \
   gsl_histogram__stat2d.c gsl_ieee-utils__env.c gsl_ieee-utils__fp.c \
   gsl_ieee-utils__make_rep.c gsl_ieee-utils__print.c gsl_ieee-utils__read.c \
   gsl_integration__qag.c gsl_integration__qagp.c gsl_integration__qags.c \
   gsl_integration__qawc.c gsl_integration__qawf.c gsl_integration__qawo.c \
   gsl_integration__qaws.c gsl_integration__qcheb.c gsl_integration__qk.c \
   gsl_integration__qk15.c gsl_integration__qk21.c gsl_integration__qk31.c \
   gsl_integration__qk41.c gsl_integration__qk51.c gsl_integration__qk61.c \
   gsl_integration__qmomo.c gsl_integration__qmomof.c gsl_integration__qng.c \
   gsl_integration__workspace.c gsl_linalg__balance.c gsl_linalg__balancemat.c \
   gsl_linalg__bidiag.c gsl_linalg__cholesky.c gsl_linalg__choleskyc.c \
   gsl_linalg__exponential.c gsl_linalg__hermtd.c gsl_linalg__hessenberg.c \
   gsl_linalg__hesstri.c gsl_linalg__hh.c gsl_linalg__householder.c \
   gsl_linalg__householdercomplex.c gsl_linalg__lq.c gsl_linalg__lu.c \
   gsl_linalg__luc.c gsl_linalg__multiply.c gsl_linalg__ptlq.c \
   gsl_linalg__qr.c gsl_linalg__qrpt.c gsl_linalg__svd.c \
   gsl_linalg__symmtd.c gsl_linalg__tridiag.c gsl_matrix__copy.c \
   gsl_matrix__file.c gsl_matrix__getset.c gsl_matrix__init.c \
   gsl_matrix__matrix.c gsl_matrix__minmax.c gsl_matrix__oper.c \
   gsl_matrix__prop.c gsl_matrix__rowcol.c gsl_matrix__submatrix.c \
   gsl_matrix__swap.c gsl_matrix__view.c gsl_min__bracketing.c \
   gsl_min__brent.c gsl_min__convergence.c gsl_min__fsolver.c \
   gsl_min__golden.c gsl_monte__miser.c gsl_monte__plain.c \
   gsl_monte__vegas.c gsl_multifit__convergence.c gsl_multifit__covar.c \
   gsl_multifit__fdfsolver.c gsl_multifit__fsolver.c gsl_multifit__gradient.c \
   gsl_multifit__lmder.c gsl_multifit__multilinear.c gsl_multifit__work.c \
   gsl_multimin__conjugate_fr.c gsl_multimin__conjugate_pr.c gsl_multimin__convergence.c \
   gsl_multimin__diff.c gsl_multimin__fdfminimizer.c gsl_multimin__fminimizer.c \
   gsl_multimin__simplex.c gsl_multimin__steepest_descent.c gsl_multimin__vector_bfgs.c \
   gsl_multimin__vector_bfgs2.c gsl_multiroots__broyden.c gsl_multiroots__convergence.c \
   gsl_multiroots__dnewton.c gsl_multiroots__fdfsolver.c gsl_multiroots__fdjac.c \
   gsl_multiroots__fsolver.c gsl_multiroots__gnewton.c gsl_multiroots__hybrid.c \
   gsl_multiroots__hybridj.c gsl_multiroots__newton.c gsl_ntuple__ntuple.c \
   gsl_ode-initval__bsimp.c gsl_ode-initval__control.c gsl_ode-initval__cscal.c \
   gsl_ode-initval__cstd.c gsl_ode-initval__evolve.c gsl_ode-initval__gear1.c \
   gsl_ode-initval__gear2.c gsl_ode-initval__rk2.c gsl_ode-initval__rk2imp.c \
   gsl_ode-initval__rk2simp.c gsl_ode-initval__rk4.c gsl_ode-initval__rk4imp.c \
   gsl_ode-initval__rk8pd.c gsl_ode-initval__rkck.c gsl_ode-initval__rkf45.c \
   gsl_ode-initval__step.c gsl_permutation__canonical.c gsl_permutation__file.c \
   gsl_permutation__init.c gsl_permutation__permutation.c gsl_permutation__permute.c \
   gsl_poly__dd.c gsl_poly__eval.c gsl_poly__solve_cubic.c \
   gsl_poly__solve_quadratic.c gsl_poly__zsolve.c gsl_poly__zsolve_cubic.c \
   gsl_poly__zsolve_init.c gsl_poly__zsolve_quadratic.c gsl_qrng__niederreiter-2.c \
   gsl_qrng__qrng.c gsl_qrng__sobol.c gsl_randist__bernoulli.c \
   gsl_randist__beta.c gsl_randist__bigauss.c gsl_randist__binomial.c \
   gsl_randist__binomial_tpe.c gsl_randist__cauchy.c gsl_randist__chisq.c \
   gsl_randist__dirichlet.c gsl_randist__discrete.c gsl_randist__erlang.c \
   gsl_randist__exponential.c gsl_randist__exppow.c gsl_randist__fdist.c \
   gsl_randist__flat.c gsl_randist__gamma.c gsl_randist__gauss.c \
   gsl_randist__gausstail.c gsl_randist__gausszig.c gsl_randist__geometric.c \
   gsl_randist__gumbel.c gsl_randist__hyperg.c gsl_randist__landau.c \
   gsl_randist__laplace.c gsl_randist__levy.c gsl_randist__logarithmic.c \
   gsl_randist__logistic.c gsl_randist__lognormal.c gsl_randist__multinomial.c \
   gsl_randist__nbinomial.c gsl_randist__pareto.c gsl_randist__pascal.c \
   gsl_randist__poisson.c gsl_randist__rayleigh.c gsl_randist__shuffle.c \
   gsl_randist__sphere.c gsl_randist__tdist.c gsl_randist__weibull.c \
   gsl_rng__borosh13.c gsl_rng__cmrg.c gsl_rng__coveyou.c \
   gsl_rng__default.c gsl_rng__file.c gsl_rng__fishman18.c \
   gsl_rng__fishman20.c gsl_rng__fishman2x.c gsl_rng__gfsr4.c \
   gsl_rng__knuthran.c gsl_rng__knuthran2.c gsl_rng__knuthran2002.c \
   gsl_rng__lecuyer21.c gsl_rng__minstd.c gsl_rng__mrg.c \
   gsl_rng__mt.c gsl_rng__r250.c gsl_rng__ran0.c \
   gsl_rng__ran1.c gsl_rng__ran2.c gsl_rng__ran3.c \
   gsl_rng__rand.c gsl_rng__rand48.c gsl_rng__random.c \
   gsl_rng__randu.c gsl_rng__ranf.c gsl_rng__ranlux.c \
   gsl_rng__ranlxd.c gsl_rng__ranlxs.c gsl_rng__ranmar.c \
   gsl_rng__rng.c gsl_rng__slatec.c gsl_rng__taus.c \
   gsl_rng__taus113.c gsl_rng__transputer.c gsl_rng__tt.c \
   gsl_rng__types.c gsl_rng__uni.c gsl_rng__uni32.c \
   gsl_rng__vax.c gsl_rng__waterman14.c gsl_rng__zuf.c \
   gsl_roots__bisection.c gsl_roots__brent.c gsl_roots__convergence.c \
   gsl_roots__falsepos.c gsl_roots__fdfsolver.c gsl_roots__fsolver.c \
   gsl_roots__newton.c gsl_roots__secant.c gsl_roots__steffenson.c \
   gsl_siman__siman.c gsl_sort__sort.c gsl_sort__sortind.c \
   gsl_sort__sortvec.c gsl_sort__sortvecind.c gsl_sort__subset.c \
   gsl_sort__subsetind.c gsl_specfunc__airy.c gsl_specfunc__airy_der.c \
   gsl_specfunc__airy_zero.c gsl_specfunc__atanint.c gsl_specfunc__bessel.c \
   gsl_specfunc__bessel_I0.c gsl_specfunc__bessel_I1.c gsl_specfunc__bessel_In.c \
   gsl_specfunc__bessel_Inu.c gsl_specfunc__bessel_J0.c gsl_specfunc__bessel_J1.c \
   gsl_specfunc__bessel_Jn.c gsl_specfunc__bessel_Jnu.c gsl_specfunc__bessel_K0.c \
   gsl_specfunc__bessel_K1.c gsl_specfunc__bessel_Kn.c gsl_specfunc__bessel_Knu.c \
   gsl_specfunc__bessel_Y0.c gsl_specfunc__bessel_Y1.c gsl_specfunc__bessel_Yn.c \
   gsl_specfunc__bessel_Ynu.c gsl_specfunc__bessel_amp_phase.c gsl_specfunc__bessel_i.c \
   gsl_specfunc__bessel_j.c gsl_specfunc__bessel_k.c gsl_specfunc__bessel_olver.c \
   gsl_specfunc__bessel_sequence.c gsl_specfunc__bessel_temme.c gsl_specfunc__bessel_y.c \
   gsl_specfunc__bessel_zero.c gsl_specfunc__beta.c gsl_specfunc__beta_inc.c \
   gsl_specfunc__clausen.c gsl_specfunc__coulomb.c gsl_specfunc__coulomb_bound.c \
   gsl_specfunc__coupling.c gsl_specfunc__dawson.c gsl_specfunc__debye.c \
   gsl_specfunc__dilog.c gsl_specfunc__elementary.c gsl_specfunc__ellint.c \
   gsl_specfunc__elljac.c gsl_specfunc__erfc.c gsl_specfunc__exp.c \
   gsl_specfunc__expint.c gsl_specfunc__expint3.c gsl_specfunc__fermi_dirac.c \
   gsl_specfunc__gamma.c gsl_specfunc__gamma_inc.c gsl_specfunc__gegenbauer.c \
   gsl_specfunc__hyperg.c gsl_specfunc__hyperg_0F1.c gsl_specfunc__hyperg_1F1.c \
   gsl_specfunc__hyperg_2F0.c gsl_specfunc__hyperg_2F1.c gsl_specfunc__hyperg_U.c \
   gsl_specfunc__laguerre.c gsl_specfunc__lambert.c gsl_specfunc__legendre_H3d.c \
   gsl_specfunc__legendre_Qn.c gsl_specfunc__legendre_con.c gsl_specfunc__legendre_poly.c \
   gsl_specfunc__log.c gsl_specfunc__mathieu_angfunc.c gsl_specfunc__mathieu_charv.c \
   gsl_specfunc__mathieu_coeff.c gsl_specfunc__mathieu_radfunc.c gsl_specfunc__mathieu_workspace.c \
   gsl_specfunc__poch.c gsl_specfunc__pow_int.c gsl_specfunc__psi.c \
   gsl_specfunc__result.c gsl_specfunc__shint.c gsl_specfunc__sinint.c \
   gsl_specfunc__synchrotron.c gsl_specfunc__transport.c gsl_specfunc__trig.c \
   gsl_specfunc__zeta.c gsl_statistics__absdev.c gsl_statistics__covariance.c \
   gsl_statistics__kurtosis.c gsl_statistics__lag1.c gsl_statistics__mean.c \
   gsl_statistics__median.c gsl_statistics__minmax.c gsl_statistics__p_variance.c \
   gsl_statistics__quantiles.c gsl_statistics__skew.c gsl_statistics__ttest.c \
   gsl_statistics__variance.c gsl_statistics__wabsdev.c gsl_statistics__wkurtosis.c \
   gsl_statistics__wmean.c gsl_statistics__wskew.c gsl_statistics__wvariance.c \
   gsl_sum__levin_u.c gsl_sum__levin_utrunc.c gsl_sum__work_u.c \
   gsl_sum__work_utrunc.c gsl_sys__coerce.c gsl_sys__expm1.c \
   gsl_sys__fcmp.c gsl_sys__fdiv.c gsl_sys__hypot.c \
   gsl_sys__infnan.c gsl_sys__invhyp.c gsl_sys__ldfrexp.c \
   gsl_sys__log1p.c gsl_sys__minmax.c gsl_sys__pow_int.c \
   gsl_sys__prec.c gsl_vector__copy.c gsl_vector__file.c \
   gsl_vector__init.c gsl_vector__minmax.c gsl_vector__oper.c \
   gsl_vector__prop.c gsl_vector__reim.c gsl_vector__subvector.c \
   gsl_vector__swap.c gsl_vector__vector.c gsl_vector__view.c \
   gsl_wavelet__bspline.c gsl_wavelet__daubechies.c gsl_wavelet__dwt.c \
   gsl_wavelet__haar.c gsl_wavelet__wavelet.c

HEADERS += \
    gsl__config.h \
    gsl_blas.h \
    gsl_blas_types.h \
    gsl_block.h \
    gsl_block_char.h \
    gsl_block_complex_double.h \
    gsl_block_complex_float.h \
    gsl_block_complex_long_double.h \
    gsl_block_double.h \
    gsl_block_float.h \
    gsl_block_int.h \
    gsl_block_long.h \
    gsl_block_long_double.h \
    gsl_block_short.h \
    gsl_block_uchar.h \
    gsl_block_uint.h \
    gsl_block_ulong.h \
    gsl_block_ushort.h \
    gsl_bspline.h \
    gsl_cblas.h \
    gsl_cblas__cblas.h \
    gsl_cblas__source_asum_c.h \
    gsl_cblas__source_asum_r.h \
    gsl_cblas__source_axpy_c.h \
    gsl_cblas__source_axpy_r.h \
    gsl_cblas__source_copy_c.h \
    gsl_cblas__source_copy_r.h \
    gsl_cblas__source_dot_c.h \
    gsl_cblas__source_dot_r.h \
    gsl_cblas__source_gbmv_c.h \
    gsl_cblas__source_gbmv_r.h \
    gsl_cblas__source_gemm_c.h \
    gsl_cblas__source_gemm_r.h \
    gsl_cblas__source_gemv_c.h \
    gsl_cblas__source_gemv_r.h \
    gsl_cblas__source_ger.h \
    gsl_cblas__source_gerc.h \
    gsl_cblas__source_geru.h \
    gsl_cblas__source_hbmv.h \
    gsl_cblas__source_hemm.h \
    gsl_cblas__source_hemv.h \
    gsl_cblas__source_her.h \
    gsl_cblas__source_her2.h \
    gsl_cblas__source_her2k.h \
    gsl_cblas__source_herk.h \
    gsl_cblas__source_hpmv.h \
    gsl_cblas__source_hpr.h \
    gsl_cblas__source_hpr2.h \
    gsl_cblas__source_iamax_c.h \
    gsl_cblas__source_iamax_r.h \
    gsl_cblas__source_nrm2_c.h \
    gsl_cblas__source_nrm2_r.h \
    gsl_cblas__source_rot.h \
    gsl_cblas__source_rotg.h \
    gsl_cblas__source_rotm.h \
    gsl_cblas__source_rotmg.h \
    gsl_cblas__source_sbmv.h \
    gsl_cblas__source_scal_c.h \
    gsl_cblas__source_scal_c_s.h \
    gsl_cblas__source_scal_r.h \
    gsl_cblas__source_spmv.h \
    gsl_cblas__source_spr.h \
    gsl_cblas__source_spr2.h \
    gsl_cblas__source_swap_c.h \
    gsl_cblas__source_swap_r.h \
    gsl_cblas__source_symm_c.h \
    gsl_cblas__source_symm_r.h \
    gsl_cblas__source_symv.h \
    gsl_cblas__source_syr.h \
    gsl_cblas__source_syr2.h \
    gsl_cblas__source_syr2k_c.h \
    gsl_cblas__source_syr2k_r.h \
    gsl_cblas__source_syrk_c.h \
    gsl_cblas__source_syrk_r.h \
    gsl_cblas__source_tbmv_c.h \
    gsl_cblas__source_tbmv_r.h \
    gsl_cblas__source_tbsv_c.h \
    gsl_cblas__source_tbsv_r.h \
    gsl_cblas__source_tpmv_c.h \
    gsl_cblas__source_tpmv_r.h \
    gsl_cblas__source_tpsv_c.h \
    gsl_cblas__source_tpsv_r.h \
    gsl_cblas__source_trmm_c.h \
    gsl_cblas__source_trmm_r.h \
    gsl_cblas__source_trmv_c.h \
    gsl_cblas__source_trmv_r.h \
    gsl_cblas__source_trsm_c.h \
    gsl_cblas__source_trsm_r.h \
    gsl_cblas__source_trsv_c.h \
    gsl_cblas__source_trsv_r.h \
    gsl_cdf.h \
    gsl_cdf__error.h \
    gsl_cdf__rat_eval.h \
    gsl_chebyshev.h \
    gsl_check_range.h \
    gsl_combination.h \
    gsl_complex.h \
    gsl_complex__results.h \
    gsl_complex__results_real.h \
    gsl_complex__results1.h \
    gsl_complex__results2.h \
    gsl_complex_math.h \
    gsl_const.h \
    gsl_const_cgs.h \
    gsl_const_cgsm.h \
    gsl_const_mks.h \
    gsl_const_mksa.h \
    gsl_const_num.h \
    gsl_deriv.h \
    gsl_dft_complex.h \
    gsl_dft_complex_float.h \
    gsl_dht.h \
    gsl_diff.h \
    gsl_eigen.h \
    gsl_errno.h \
    gsl_fft.h \
    gsl_fft__bitreverse.h \
    gsl_fft__c_pass.h \
    gsl_fft__compare.h \
    gsl_fft__complex_internal.h \
    gsl_fft__factorize.h \
    gsl_fft__hc_pass.h \
    gsl_fft__real_pass.h \
    gsl_fft__signals.h \
    gsl_fft_complex.h \
    gsl_fft_complex_float.h \
    gsl_fft_halfcomplex.h \
    gsl_fft_halfcomplex_float.h \
    gsl_fft_real.h \
    gsl_fft_real_float.h \
    gsl_fit.h \
    gsl_heapsort.h \
    gsl_histogram.h \
    gsl_histogram2d.h \
    gsl_ieee_utils.h \
    gsl_integration.h \
    gsl_integration__qng.h \
    gsl_interp.h \
    gsl_linalg.h \
    gsl_linalg__tridiag.h \
    gsl_machine.h \
    gsl_math.h \
    gsl_matrix.h \
    gsl_matrix__view.h \
    gsl_matrix_char.h \
    gsl_matrix_complex_double.h \
    gsl_matrix_complex_float.h \
    gsl_matrix_complex_long_double.h \
    gsl_matrix_double.h \
    gsl_matrix_float.h \
    gsl_matrix_int.h \
    gsl_matrix_long.h \
    gsl_matrix_long_double.h \
    gsl_matrix_short.h \
    gsl_matrix_uchar.h \
    gsl_matrix_uint.h \
    gsl_matrix_ulong.h \
    gsl_matrix_ushort.h \
    gsl_message.h \
    gsl_min.h \
    gsl_min__min.h \
    gsl_mode.h \
    gsl_monte.h \
    gsl_monte_miser.h \
    gsl_monte_plain.h \
    gsl_monte_vegas.h \
    gsl_multifit.h \
    gsl_multifit_nlin.h \
    gsl_multimin.h \
    gsl_multiroots.h \
    gsl_nan.h \
    gsl_ntuple.h \
    gsl_ode-initval__odeiv_util.h \
    gsl_odeiv.h \
    gsl_permutation.h \
    gsl_permute.h \
    gsl_permute_char.h \
    gsl_permute_complex_double.h \
    gsl_permute_complex_float.h \
    gsl_permute_complex_long_double.h \
    gsl_permute_double.h \
    gsl_permute_float.h \
    gsl_permute_int.h \
    gsl_permute_long.h \
    gsl_permute_long_double.h \
    gsl_permute_short.h \
    gsl_permute_uchar.h \
    gsl_permute_uint.h \
    gsl_permute_ulong.h \
    gsl_permute_ushort.h \
    gsl_permute_vector.h \
    gsl_permute_vector_char.h \
    gsl_permute_vector_complex_double.h \
    gsl_permute_vector_complex_float.h \
    gsl_permute_vector_complex_long_double.h \
    gsl_permute_vector_double.h \
    gsl_permute_vector_float.h \
    gsl_permute_vector_int.h \
    gsl_permute_vector_long.h \
    gsl_permute_vector_long_double.h \
    gsl_permute_vector_short.h \
    gsl_permute_vector_uchar.h \
    gsl_permute_vector_uint.h \
    gsl_permute_vector_ulong.h \
    gsl_permute_vector_ushort.h \
    gsl_poly.h \
    gsl_pow_int.h \
    gsl_precision.h \
    gsl_qrng.h \
    gsl_randist.h \
    gsl_rng.h \
    gsl_roots.h \
    gsl_roots__roots.h \
    gsl_sf.h \
    gsl_sf_airy.h \
    gsl_sf_bessel.h \
    gsl_sf_clausen.h \
    gsl_sf_coulomb.h \
    gsl_sf_coupling.h \
    gsl_sf_dawson.h \
    gsl_sf_debye.h \
    gsl_sf_dilog.h \
    gsl_sf_elementary.h \
    gsl_sf_ellint.h \
    gsl_sf_elljac.h \
    gsl_sf_erf.h \
    gsl_sf_exp.h \
    gsl_sf_expint.h \
    gsl_sf_fermi_dirac.h \
    gsl_sf_gamma.h \
    gsl_sf_gegenbauer.h \
    gsl_sf_hyperg.h \
    gsl_sf_laguerre.h \
    gsl_sf_lambert.h \
    gsl_sf_legendre.h \
    gsl_sf_log.h \
    gsl_sf_mathieu.h \
    gsl_sf_pow_int.h \
    gsl_sf_psi.h \
    gsl_sf_result.h \
    gsl_sf_synchrotron.h \
    gsl_sf_transport.h \
    gsl_sf_trig.h \
    gsl_sf_zeta.h \
    gsl_siman.h \
    gsl_sort.h \
    gsl_sort_char.h \
    gsl_sort_double.h \
    gsl_sort_float.h \
    gsl_sort_int.h \
    gsl_sort_long.h \
    gsl_sort_long_double.h \
    gsl_sort_short.h \
    gsl_sort_uchar.h \
    gsl_sort_uint.h \
    gsl_sort_ulong.h \
    gsl_sort_ushort.h \
    gsl_sort_vector.h \
    gsl_sort_vector_char.h \
    gsl_sort_vector_double.h \
    gsl_sort_vector_float.h \
    gsl_sort_vector_int.h \
    gsl_sort_vector_long.h \
    gsl_sort_vector_long_double.h \
    gsl_sort_vector_short.h \
    gsl_sort_vector_uchar.h \
    gsl_sort_vector_uint.h \
    gsl_sort_vector_ulong.h \
    gsl_sort_vector_ushort.h \
    gsl_specfunc.h \
    gsl_specfunc__bessel.h \
    gsl_specfunc__bessel_amp_phase.h \
    gsl_specfunc__bessel_olver.h \
    gsl_specfunc__bessel_temme.h \
    gsl_specfunc__chebyshev.h \
    gsl_specfunc__check.h \
    gsl_specfunc__error.h \
    gsl_specfunc__eval.h \
    gsl_specfunc__hyperg.h \
    gsl_specfunc__legendre.h \
    gsl_specfunc__recurse.h \
    gsl_spline.h \
    gsl_statistics.h \
    gsl_statistics_char.h \
    gsl_statistics_double.h \
    gsl_statistics_float.h \
    gsl_statistics_int.h \
    gsl_statistics_long.h \
    gsl_statistics_long_double.h \
    gsl_statistics_short.h \
    gsl_statistics_uchar.h \
    gsl_statistics_uint.h \
    gsl_statistics_ulong.h \
    gsl_statistics_ushort.h \
    gsl_sum.h \
    gsl_sys.h \
    gsl_test.h \
    gsl_types.h \
    gsl_vector.h \
    gsl_vector__view.h \
    gsl_vector_char.h \
    gsl_vector_complex.h \
    gsl_vector_complex_double.h \
    gsl_vector_complex_float.h \
    gsl_vector_complex_long_double.h \
    gsl_vector_double.h \
    gsl_vector_float.h \
    gsl_vector_int.h \
    gsl_vector_long.h \
    gsl_vector_long_double.h \
    gsl_vector_short.h \
    gsl_vector_uchar.h \
    gsl_vector_uint.h \
    gsl_vector_ulong.h \
    gsl_vector_ushort.h \
    gsl_version.h \
    gsl_wavelet.h \
    gsl_wavelet2d.h \
    templates_off.h \
    templates_on.h
