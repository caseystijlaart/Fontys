class TestCase {
	String name
	int points
}

class ExecResult {
	int exitValue
	StringBuilder out = new StringBuilder()
	StringBuilder err = new StringBuilder()
	boolean crashed
}

ExecResult runCmd(String cmd) {
	ExecResult r = new ExecResult()
	r.crashed = false

	def sout = new StringBuilder(), serr = new StringBuilder()
	def proc = cmd.execute()
	proc.consumeProcessOutput(r.out, r.err)
	proc.waitForOrKill(5000)
	r.exitValue = proc.exitValue()

	if(r.exitValue == 139) {
		r.crashed = true
	}

	return r
}

Integer queryNumberOfTestCases() {
	ExecResult r = runCmd("./main_test size")
	if(r.exitValue == 0) {
		return r.out.toInteger()
	}
	return 0
}

TestCase queryTestCase(Integer index) {
	ExecResult r = runCmd("./main_test info $index")
	TestCase t 

	if(r.exitValue == 0) {
		t = new TestCase()
		def splits = r.out.toString().split(",").collect{it.trim() }
		t.name = splits[0]
		t.points = splits[1].toInteger()
	}

	return t
}

/*------------------------------------------------------------------*/
def testCases = []
def totalPoints = 0
def failed = 0
def succeeded = 0

queryNumberOfTestCases().times{ index -> 
	testCases << queryTestCase(index)
}

def maxPoints = testCases.sum{ it.points }

if(testCases.size() == 0) { println "No testCases present "}

println " | Test-case | Score"
println "---|---|---"
testCases.eachWithIndex{ testCase, index ->
	def cmd = "./main_test run $index"

	ExecResult execResult = runCmd(cmd)
	
	if(execResult.crashed) {
		failed++
		testCase.points = 0
	}
	else {
		if(execResult.exitValue == testCase.points) {
			succeeded++
		}
		else {
			failed++
			testCase.points = execResult.exitValue
		}
	}

	totalPoints += testCase.points
	def crashReport = execResult.crashed ? " | Crashed" : ""
	println "${index} | ${testCase.name} | ${testCase.points} $crashReport"
}

println " | PASSED: $succeeded, FAILED: $failed"
println "Score | $totalPoints of $maxPoints"
