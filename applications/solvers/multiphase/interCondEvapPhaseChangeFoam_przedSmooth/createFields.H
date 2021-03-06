Info<< "Reading field p_rgh\n" << endl;
volScalarField p_rgh
(
    IOobject
    (
        "p_rgh",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

Info<< "Reading field T\n" << endl;
volScalarField T
(
    IOobject
    (
        "T",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

Info<< "Reading field U\n" << endl;
volVectorField U
(
    IOobject
    (
        "U",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

#include "createPhi.H"


Info<< "Creating phaseChangeTwoPhaseMixture\n" << endl;
autoPtr<phaseChangeTwoPhaseMixture> mixture =
    phaseChangeTwoPhaseMixture::New(U, phi);

volScalarField& alpha1(mixture->alpha1());
volScalarField& alpha2(mixture->alpha2());

const dimensionedScalar& rho1 = mixture->rho1();
const dimensionedScalar& rho2 = mixture->rho2();
const dimensionedScalar& pSat = mixture->pSat();
const dimensionedScalar& cutoff = mixture->cutoff();
const dimensionedScalar TSat("TSat", dimensionSet(0,0,0,1,0,0,0), 380.26);
const dimensionedScalar hEvap("hEvap", dimensionSet(0,2,-2,0,0,0,0), 2237.41e3);
const dimensionedScalar k1("k1", dimensionSet(1,1,-3,-1,0,0,0), 0.68106);
const dimensionedScalar k2("k2", dimensionSet(1,1,-3,-1,0,0,0), 0.025905);
const dimensionedScalar cp1("cp1", dimensionSet(0,2,-2,-1,0,0,0), 4224);
const dimensionedScalar cp2("cp2", dimensionSet(0,2,-2,-1,0,0,0), 2110);

// Need to store rho for ddt(rho, U)
volScalarField rho
(
    IOobject
    (
        "rho",
        runTime.timeName(),
        mesh,
        IOobject::READ_IF_PRESENT
    ),
    alpha1*rho1 + alpha2*rho2
);
rho.oldTime();

//    surfaceScalarField rhoPhi
//    (
//        IOobject
//        (
//            "rhoPhi",
//            runTime.timeName(),
//            mesh,
//            IOobject::NO_READ,
//            IOobject::NO_WRITE
//        ),
//        rho1*phi
//    );

    volScalarField mCond
    (
        IOobject
        (
            "mCond",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mixture->mDot()[0]
    );

    volScalarField mVap
    (
        IOobject
        (
            "mVap",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mixture->mDot()[1]
    );
// Construct interface from alpha1 distribution
interfaceProperties interface(alpha1, U, mixture());

// Construct incompressible turbulence model
autoPtr<incompressible::turbulenceModel> turbulence
(
    incompressible::turbulenceModel::New(U, phi, mixture())
);


#include "readGravitationalAcceleration.H"
#include "readhRef.H"
#include "gh.H"


volScalarField p
(
    IOobject
    (
        "p",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    p_rgh + rho*gh
);

label pRefCell = 0;
scalar pRefValue = 0.0;
setRefCell
(
    p,
    p_rgh,
    pimple.dict(),
    pRefCell,
    pRefValue
);

if (p_rgh.needReference())
{
    p += dimensionedScalar
    (
        "p",
        p.dimensions(),
        pRefValue - getRefCellValue(p, pRefCell)
    );
    p_rgh = p - rho*gh;
}

mesh.setFluxRequired(p_rgh.name());
mesh.setFluxRequired(alpha1.name());
