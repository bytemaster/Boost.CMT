#include <boost/cmt/actor.hpp>
#include <boost/cmt/log/log.hpp>


class calculator {
  public:
    calculator():r(0){};
    int add( int v ) { slog( "%1%", v ); return r += v; }

  private:
    int r;
};

BOOST_REFLECT_ANY( calculator, (add) )


void test( boost::cmt::actor<calculator> act ) {
  slog( "start act test" );
  slog( "+5] %1%", (int)act->add( 5 ));
  slog( "+6] %1%", (int)act->add( 6 ).wait() );
  slog( "+7] %1%", (int)act->add( 7 ).wait() );
}

int main( int argc, char** argv ) {

  boost::cmt::thread* at = boost::cmt::thread::create( "actor_thread" );
  boost::reflect::any_ptr<calculator> ap( boost::make_shared<calculator>() );
  boost::cmt::actor<calculator> act( boost::make_shared<calculator>(), at );
  boost::cmt::actor<calculator> act2(ap, at );

  boost::cmt::thread* tt = boost::cmt::thread::create( "test_thread" );
  tt->async<void>( boost::bind(test, act) ).wait();
  tt->async<void>( boost::bind(test, act2) ).wait();
  at->async<void>( boost::bind(test, act2) ).wait();

  //test(act);
  test(act2);

  tt->quit();
  at->quit();

  return 0;
}
